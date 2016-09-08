#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QHostAddress>
#include <QMessageBox>
#include <QTextStream>
#include <QByteArray>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QKeyEvent>
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void keyPressEvent(QKeyEvent *e);
     void keyReleaseEvent(QKeyEvent *e);
    void slotConnect();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError);
    void slotReadData();
    void on_pushButton_2_pressed();

    void on_pushButton_2_released();

    void on_pushButton_left_pressed();

    void on_pushButton_left_released();

    void on_pushButton_right_pressed();

    void on_pushButton_right_released();

    void on_pushButton_back_pressed();

    void on_pushButton_back_released();

    void on_pushButton_bibi_clicked();

private:

    Ui::Widget *ui;

    QTcpSocket *socket;
    QUdpSocket *udpsock;
    int udpport;

    int flag;
    int framlenth;
};

#endif // WIDGET_H
