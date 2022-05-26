#ifndef CHECKDATA_H
#define CHECKDATA_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
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
    QTime *myTime_ch1;
    QTime *myTime_ch2;
    QString command;
    bool flagPackage;
    bool flagChannel_1;
    bool flagChannel_2;
    bool flagNumPackage;
    int numPackage;
    int numBit;
    quint16 byteRecieveSync_CH1;
    quint16 byteRecieveSync_CH2;
    quint16 byteMarkerSync_CH1;
    quint16 byteMarkerSync_CH2;
    QByteArray Channel1;
    QByteArray Channel2;
    QByteArray byteEtalon;
    bool flagSyncCh1;
    bool flagSyncCh2;
    int countShift_ch1;
    int countShift_ch2;
    int countValidity_Ch1;
    int countValidity_Ch2;
    double validity_1;
    int validityTrue_1;
    double validityAll_1;
    double validity_2;
    int validityTrue_2;
    double validityAll_2;
    bool pushRead;
    QString save_strData;



private slots:
    void openPort();
    void closePort();
    void setRate_slot(int rate);
    void reset_Arduino();
    void parsingPackage(QByteArray data);
    void writePort(QByteArray data);
    QByteArray readPort();
    void openPatternFile();
    void validitySignal(int numChannel, QByteArray byte_msg);
    void writeFileMSG(int numChannel, QByteArray msg);
    void clearFileMSG();
    void alarmMSG();
    void debugTextEdit(bool status, QString debMSG);
    void reset_Telementry();
    void clear_LogDialog();
    void slot_StartRead();
    void slot_StopRead();
};

#endif // CHECKDATA_H
