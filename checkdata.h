#ifndef CHECKDATA_H
#define CHECKDATA_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QTime>


namespace Ui {
class CheckData;
}

class CheckData : public QMainWindow
{
    Q_OBJECT

public:
    explicit CheckData(QWidget *parent = 0);
    ~CheckData();

private:
    Ui::CheckData *ui;
    QSerialPort *port;
    QString command;
    bool flagPackage;
    bool flagChannel_1;
    bool flagChannel_2;
    bool flagNumPackage;
    int numPackage;

private slots:
    void openPort();
    void closePort();
    void setRate_slot(int rate);
    void receiveMsg(QByteArray data);
    void readPort();
    void reset_Arduino();
    void writePort(QByteArray data);

};

#endif // CHECKDATA_H
