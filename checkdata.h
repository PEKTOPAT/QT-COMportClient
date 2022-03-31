#ifndef CHECKDATA_H
#define CHECKDATA_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QTime>
#include <QTimer>


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
    QString Pattern;
    QByteArray dataforSend;
    int count;
    QTimer *timer;



private slots:
    void openPort();
    void closePort();
    void setRate_slot(int rate);
    void writePort();
    void openPatternFile();
    void debugTextEdit(bool status, QString debMSG);
    void createPackage();
    void sendClick();
    void stopSendClick();

};

#endif // CHECKDATA_H
