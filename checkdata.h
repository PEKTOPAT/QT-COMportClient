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

private slots:
    void openPort();
    void closePort();
    void setRate_slot(int rate);

};

#endif // CHECKDATA_H
