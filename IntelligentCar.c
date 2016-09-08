#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <core.h>
#include <pthread.h>
#define SIREN 			3

#define TRIG			4
#define ECHO			5

//left
#define MOTO_LEFT_S		9  //speed
#define MOTO_LEFT_1	 	8   //forward
#define MOTO_LEFT_2 	11  //backward

//right
#define MOTO_RIGHT_S 	10  //speed
#define MOTO_RIGHT_1	12  //forward
#define MOTO_RIGHT_2	13  //backward

#define SOCK_HOST 		""
#define SOCK_PORT		9999

#define BUFSIZE  		128
#define MOTOSIZE		5
#define CTLSIZE			6

const char motoctrl_forward[CTLSIZE] = {
	20,1,0,20,1,0
};
const char motoctrl_backward[CTLSIZE] = {
	15,0,1,15,0,1
};
const char motoctrl_turnleft[CTLSIZE] = {
	10,1,0,10,0,1
};

const char motoctrl_turnright[CTLSIZE] = {
	10,0,1,10,1,0
};

const char motoctrl_stop[CTLSIZE]  ={
	0,0,0,0,0,0
};
typedef struct{
	char cmd;
	const char *motoctrl;
}motoctrl_t;

motoctrl_t motoctrl_table[MOTOSIZE] = {
	{'w',&motoctrl_forward[0]},
	{'x',&motoctrl_backward[0]},
	{'a',&motoctrl_turnleft[0]},
	{'d',&motoctrl_turnright[0]},
	{'s',&motoctrl_stop[0]}
};


int sockfd;
pthread_t tid;
pthread_t tid2;
volatile char recvbuf[BUFSIZE];
volatile float distance;
int flag_go;

void moto_ctrl(const char *);
void moto_init(void);
void udp_init(void);
void siren_init(void);
void siren_ctrl(void);
void utrlsound_init(void);
void *utrlsound_ctrl(void *);

void setup()
{
	siren_init();
	moto_init();
	udp_init();
	utrlsound_init();
}

void loop()
{
	fd_set rdset;
	int maxfd = sockfd + 1;
	while(1)
	{
		FD_ZERO(&rdset);
		FD_SET(sockfd,&rdset);
		select(maxfd,&rdset,NULL,NULL,NULL);
		printf("read :");
		if(FD_ISSET(sockfd,&rdset))
		{
			recvfrom(sockfd,(void *)recvbuf,BUFSIZE,0,NULL,NULL);
			printf("%s\n",recvbuf);
			if(distance < 0.2 && recvbuf[0] == 'w')
			{
				continue;
			}
			for(int i=0;i < MOTOSIZE;i++)	
			{
				if('l' == recvbuf[0])
				{
					siren_ctrl();
					break;
				}
				if(motoctrl_table[i].cmd == recvbuf[0])
				{
					if(i==0)	flag_go = 1;
					else		flag_go = 0;
					moto_ctrl(motoctrl_table[i].motoctrl);
					break;
				}
			}
		}
	}
	moto_ctrl(motoctrl_stop);
	pthread_cancel(tid);
	pthread_cancel(tid2);
	close(sockfd);
}
void moto_init()
{
	pinMode(MOTO_LEFT_S,OUTPUT);
//	tone(MOTO_LEFT_S,20,1);
	pinMode(MOTO_RIGHT_S,OUTPUT);
//	tone(MOTO_RIGHT_S,20,1);
	pinMode(MOTO_LEFT_1,OUTPUT);
	pinMode(MOTO_LEFT_2,OUTPUT);
	pinMode(MOTO_RIGHT_1,OUTPUT);
	pinMode(MOTO_RIGHT_2,OUTPUT);
}

void moto_ctrl(const char *ctrl)
{
	analogWrite(MOTO_LEFT_S,ctrl[0]);
	analogWrite(MOTO_RIGHT_S,ctrl[3]);
	digitalWrite(MOTO_LEFT_1,ctrl[1]);
	digitalWrite(MOTO_LEFT_2,ctrl[2]);
	digitalWrite(MOTO_RIGHT_1,ctrl[4]);
	digitalWrite(MOTO_RIGHT_2,ctrl[5]);
}

void udp_init()
{
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0){
		printf("socket error!");
		exit(1);
	}

	struct sockaddr_in udpaddr;
	udpaddr.sin_family = AF_INET;
	udpaddr.sin_port = htons(SOCK_PORT);
	udpaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sockfd,(struct sockaddr *)&udpaddr,sizeof(udpaddr));
}


void siren_init(void)
{
	pinMode(SIREN,OUTPUT);
}

static void *siren_func(void *arg)
{
	digitalWrite(SIREN,HIGH);
	delay(500);
	digitalWrite(SIREN,LOW);
	delay(500);
	digitalWrite(SIREN,HIGH);
	delay(1000);
	digitalWrite(SIREN,LOW);
	pthread_exit((void *)0);
}

void siren_ctrl(void)
{
	pthread_create(&tid,NULL,siren_func,NULL);
	pthread_detach(tid);
	return;
}


void utrlsound_init(void)
{
	pinMode(TRIG,OUTPUT);
	pinMode(ECHO,INPUT);

	pthread_create(&tid2,NULL,utrlsound_ctrl,NULL);
	pthread_detach(tid2);
	return;
}
void *utrlsound_ctrl(void *arg)
{
	while(1)
	{
		digitalWrite(TRIG,HIGH);
		delay(2);
		digitalWrite(TRIG,LOW);
		unsigned long time = pulseIn(ECHO,HIGH,1000000);
		distance  =(volatile float) ((float)time/2/1000000 * 340);
//		printf("distance : %f\n",distance);
		if(distance < 0.2 && flag_go)
		{
			moto_ctrl(motoctrl_stop);
			flag_go = 0;
		}
		delay(100);
	}
}
