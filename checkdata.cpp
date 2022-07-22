#include "checkdata.h"
#include "ui_checkdata.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

#define longMsecPaint 42

//******************************************************************************
CheckData::CheckData(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::CheckData)
{
    ui->setupUi(this);
    setGeometry(300, 300, 480, 350);
    num_port = QSerialPortInfo::availablePorts().length();
    for(int i = 0; i < num_port; i++)
    {
        ui->comboBox->addItem(QSerialPortInfo::availablePorts().at(i).portName());
    }
    file_ch_1.setFileName("file_1.txt");
    file_ch_2.setFileName("file_2.txt");
    flagPackage = false;
    flagNumPackage = false;
    flagChannel_1 = false;
    flagChannel_2 = false;
    numPackage = 0;
    numBit = 0;
    flagSyncCh1 = false;
    flagSyncCh2 = false;
    countShift_ch1 = 0;
    countShift_ch2 = 0;
    countValidity_Ch1 = 2;
    countValidity_Ch2 = 2;
    byteRecieveSync_CH1 = 0;
    byteRecieveSync_CH2 = 0;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;
    save_strData = "";
    pushRead = true;
    timer_RefrashPort = new QTimer();
    timer_RefrashPort->start(3000);

    //    ui->progressBar_1->setValue(0);
    //    ui->progressBar_2->setValue(0);
    ui->comboBox_2->addItem("115200");
    ui->label_info_sync1->setText("Sync Ch1");
    ui->label_info_sync1->setStyleSheet("QLabel {font-weight: bold; color : red; }");
    ui->label_info_sync2->setText("Sync Ch2");
    ui->label_info_sync2->setStyleSheet("QLabel {font-weight: bold; color : red; }");

    myTime_ch1 = new QTime();
    myTime_ch2 = new QTime();
    port = new QSerialPort(this);
    port->setDataBits(QSerialPort::Data8);
    port->setFlowControl(QSerialPort::NoFlowControl);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setBaudRate(QSerialPort::Baud115200);
    //connecting
    connect(ui->push_connect,SIGNAL(clicked()),this, SLOT(openPort()));
    connect(ui->push_disconnect,SIGNAL(clicked()),this, SLOT(closePort()));
    connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRate_slot(int)));
    connect(port, SIGNAL(readyRead()), this, SLOT(readPort()));
    //connect(ui->push_reset_arduin, SIGNAL(clicked(bool)), this, SLOT(reset_Arduino()));
    connect(ui->push_reset_tlm, SIGNAL(clicked(bool)), this, SLOT(reset_Telementry()));
    connect(ui->push_download, SIGNAL(clicked(bool)), this, SLOT(openPatternFile()));
    connect(ui->push_clear_FileLog, SIGNAL(clicked(bool)), this, SLOT(clearFileMSG()));
    connect(ui->push_connect,SIGNAL(clicked()),this, SLOT(alarmMSG()));
    connect(ui->push_clear_log, SIGNAL(clicked(bool)), this, SLOT(clear_LogDialog()));
    connect(ui->push_start, SIGNAL(clicked(bool)),this,SLOT(slot_StartRead()));
    connect(ui->push_stop, SIGNAL(clicked(bool)),this,SLOT(slot_StopRead()));
    connect(timer_RefrashPort, SIGNAL(timeout()), this, SLOT(refrashPort()));
}

CheckData::~CheckData()
{
    delete ui;
    delete port;
}
//******************************************************************************
//******************************************************************************
//******************************************************************************
void CheckData::setRate_slot(int rate)
{
    if(rate == 0) port->setBaudRate(QSerialPort::Baud9600);
    else if (rate == 1) port->setBaudRate(QSerialPort::Baud19200);
    else if (rate == 2) port->setBaudRate(QSerialPort::Baud38400);
    else if (rate == 3) port->setBaudRate(QSerialPort::Baud57600);
    else if (rate == 4) port->setBaudRate(QSerialPort::Baud115200);
}
//******************************************************************************
void CheckData::openPort()
{
    if(!port) return;
    if(port->isOpen()) port->close();

    flagPackage = false;
    flagNumPackage = false;
    flagChannel_1 = false;
    flagChannel_2 = false;
    numPackage = 0;
    numBit = 0;
    flagSyncCh1 = false;
    flagSyncCh2 = false;
    Channel1.clear();
    Channel2.clear();
    countValidity_Ch1 = 2;
    countValidity_Ch2 = 2;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;
    byteRecieveSync_CH1 = 0;
    byteRecieveSync_CH2 = 0;
    countShift_ch1 = 0;
    countShift_ch2 = 0;

    port->setPortName(ui->comboBox->currentText());
    port->open(QIODevice::ReadWrite);
    if(port->isOpen())
    {
        debugTextEdit(true, "Connected");
        ui->label_status->setText("Connected");
        ui->label_status->setStyleSheet("QLabel {font-weight: bold; color : green; }");
        ui->push_connect->setEnabled(false);
        ui->push_disconnect->setEnabled(true);
        ui->label_info->setText(ui->comboBox->currentText() +" @ "+ ui->comboBox_2->currentText());
        ui->push_start->setEnabled(false);
        ui->push_stop->setEnabled(true);
        pushRead = true;
        myTime_ch1->start();
        myTime_ch2->start();
    }
    else debugTextEdit(false, "Port not open!");
}
//******************************************************************************
void CheckData::closePort()
{
    if (!port) return;
    if(port->isOpen())
    {
        port->close();
        debugTextEdit(true, "Disconnected");
        ui->label_status->setText("Disconnected");
        ui->label_status->setStyleSheet("QLabel {font-weight: bold; color : red; }");
        ui->push_connect->setEnabled(true);
        ui->push_disconnect->setEnabled(false);
        ui->label_statusPort_1->setText(" ");
        ui->label_statusPort_2->setText(" ");
        ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
        ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
        ui->label_rate_1->setText(" ");
        ui->label_rate_2->setText(" ");
        ui->label_corr_1->setText(" ");
        ui->label_corr_2->setText(" ");
        ui->label_nBit_CH1->setText(" ");
        ui->label_nBit_CH2->setText(" ");
        ui->label_nBitERR_CH1->setText(" ");
        ui->label_nBitERR_CH2->setText(" ");
        ui->push_start->setEnabled(false);
        ui->push_stop->setEnabled(false);
        pushRead = true;
        //        ui->progressBar_1->setValue(0);
        //        ui->progressBar_2->setValue(0);
        flagPackage = false;
        flagNumPackage = false;
        flagChannel_1 = false;
        flagChannel_2 = false;
        flagSyncCh1 = false;
        flagSyncCh2 = false;
        numPackage = 0;
        numBit = 0;
        Channel1.clear();
        Channel2.clear();
        countValidity_Ch1 = 2;
        countValidity_Ch2 = 2;
        validity_1 = 0;
        validity_2 = 0;
        validityTrue_1 = 0;
        validityAll_1 = 0;
        validityTrue_2 = 0;
        validityAll_2 = 0;
        byteRecieveSync_CH1 = 0;
        byteRecieveSync_CH2 = 0;
        countShift_ch1 = 0;
        countShift_ch2 = 0;
        save_strData = "";
    }
    else return;
}
//******************************************************************************
QByteArray CheckData::readPort()
{
    QByteArray data;
    if(!pushRead) return data;
    QByteArray transit;
    port->portName();
    if (port->bytesAvailable() == 0) return data;
    data = port->readAll();
    for(int i = 0; i < data.size(); i++)
    {
        transit.clear();
        transit.append(data[i]);
        parsingPackage(transit);
    }
    return data;
}
//******************************************************************************
void CheckData::parsingPackage(QByteArray data)
{
    if(data.size() == 0) return;
    const QString tab = " ";
    QString strData;
    //+++
    //    QString HEX;
    //    QString HEXmm = "0x";
    //_++
    int intData = static_cast<quint8>(data.at(0));
    for (int i = 0;i < data.size();i++)
    {
        //        HEX = QString("%1").arg(intData,0,16) + tab;
        //        HEX = HEXmm + HEX.toUpper();
        strData = strData+QString("%1").arg(intData)+tab;
    }
    //+++
    //ui->textEdit->append(HEX);
    //_++
    strData.resize(strData.length() - 1);
    //qDebug() <<"Полученное сообщение "<< strData;
    //Байт маркера начала посылки
    if(!flagPackage && numBit == 0)
    {
        if(strData == "171")
        {
            flagPackage = true;
            numBit = 1;
            //qDebug() << "Обнаружена посылка!";
        }else
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_statusPort_2->setText(" ");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_rate_1->setText(" ");
            ui->label_rate_2->setText(" ");
            debugTextEdit(false, "Error detect marker!");
            return;
        }
    }
    //Байт номера посылок, проверка на последовательность данных
    else if(!flagNumPackage && numBit == 1 && intData >= 0)
    {
        //qDebug() << "Посылка номер " << numPackage << strData;
        if(numPackage == 0 || intData == 0)
        {
            numPackage = intData;
            numBit = 2;
            flagNumPackage = true;
            return;
        }
        else if(numPackage == intData - 1)
        {
            numPackage++;
            if(numPackage == 256) numPackage = 0;
            flagNumPackage = true;
            numBit = 2;

            return;
        }
        else
        {
            flagPackage = false;
            numPackage = 0;
            numBit = 0;
            flagNumPackage = false;
            debugTextEdit(false, "Error detect num!");
            ui->label_corr_1->setText(" ");
            ui->label_corr_2->setText(" ");
            return;
        }
    }
    //Байт синхронизации
    else if(!flagChannel_1 && !flagChannel_2 && numBit == 2)
    {
        if(strData == "161")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_2->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->label_corr_2->setText(" ");
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("1,2 KB/s");
            flagChannel_1 = true;
            numBit = 3;
            return;
        }
        else if (strData == "162")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_2->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->label_corr_2->setText(" ");
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            flagChannel_1 = true;
            numBit = 3;
            return;
        }
        else if (strData == "163")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_2->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->label_corr_2->setText(" ");
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            flagChannel_1 = true;
            numBit = 3;
            return;

        }
        else if (strData == "196")
        {
            if(save_strData == strData)
            {
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText(" ");
            ui->label_rate_1->setText(" ");
            ui->label_corr_1->setText(" ");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_2->setText("1,2 KB/s");
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "200")
        {
            if(save_strData == strData)
            {
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText(" ");
            ui->label_rate_1->setText(" ");
            ui->label_corr_1->setText(" ");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "204")
        {
            if(save_strData == strData)
            {
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText(" ");
            ui->label_rate_1->setText(" ");
            ui->label_corr_1->setText(" ");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "229")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("1,2 KB/s");
            ui->label_rate_2->setText("1,2 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "230")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("1,2 KB/s");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "231")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("1,2 KB/s");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "233")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            ui->label_rate_2->setText("1,2 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "234")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("1,2 KB/s");
            ui->label_rate_2->setText("1,2 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "235")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "237")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            ui->label_rate_2->setText("1,2 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "238")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "239")
        {
            if(save_strData == strData)
            {
                flagChannel_1 = true;
                flagChannel_2 = true;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numBit = 3;
            return;
        }
        else if (strData == "128")
        {
            if(save_strData == strData)
            {
                flagPackage = false;
                flagNumPackage = false;
                numBit = 3;
                return;
            }
            save_strData = strData;
            ui->label_statusPort_1->setText(" ");
            ui->label_statusPort_2->setText(" ");
            ui->label_corr_1->setText(" ");
            ui->label_corr_2->setText(" ");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_rate_1->setText(" ");
            ui->label_rate_2->setText(" ");
            flagPackage = false;
            flagNumPackage = false;
            numBit = 3;
            return;

        }else
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_statusPort_2->setText(" ");
            ui->label_corr_1->setText(" ");
            ui->label_corr_2->setText(" ");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_rate_1->setText(" ");
            ui->label_rate_2->setText(" ");
            debugTextEdit(false, "Error sync. Wait for marker");
            flagPackage = false;
            flagNumPackage = false;
            numBit = 0;
            return;
        }
    }
    //Если получен 3 байт и поднят флаг информации 0-го, то происходит запись инф
    else if (flagChannel_1 && numBit == 3)
    {
        if(ui->checkBox_Log_full->isChecked()) writeFileMSG(1, data);
        Channel1.append(data);
        if(flagSyncCh1)
        {
            if(countShift_ch1 == 0)
            {
                if(!ui->checkBox_Log_full->isChecked()) writeFileMSG(1, data);
                validitySignal(1, data);
            }
            else
            {
                for(int i = 1; i <= (8 - countShift_ch1); i++)
                {
                    Channel1[0] = Channel1[0] << 1 | (Channel1[1] & 0x80) >> 7;
                    Channel1[1] =  Channel1[1] << 1;
                }
                Channel1[1] = Channel1[2];
                for(int i = 1; i <= countShift_ch1; i ++)
                {
                    Channel1[0] = (Channel1[0] << 1) | ((Channel1[1] & 0x80) >> 7);
                    Channel1[1] = Channel1[1] << 1;
                }
                QByteArray send;
                send.append(Channel1[0]);
                if(!ui->checkBox_Log_full->isChecked()) writeFileMSG(1, send);
                validitySignal(1, send);
                Channel1.remove(2,1);
            }
        }
        numBit = 4;
        flagChannel_1 = false;
        return;
    }
    //Если получен 3-ий байт и флаг не выставлен, записи нет, увеличиваем байт
    else if (!flagChannel_1 && numBit == 3)
    {
        numBit = 4;
        return;
    }
    //Если получен 4-ый байт и поднят флаг информации 1-го, то происходит запись инф и
    //перевод флагов в false на ожидание прихода следующего байта
    else if (numBit == 4)
    {
        if(flagChannel_2)
        {
            if(ui->checkBox_Log_full->isChecked()) writeFileMSG(2, data);
            Channel2.append(data);
            if(flagSyncCh2)
            {
                if(countShift_ch2 == 0)
                {
                    if(!ui->checkBox_Log_full->isChecked()) writeFileMSG(2, data);
                    validitySignal(2, data);
                }
                else
                {
                    for(int i = 1; i <= (8 - countShift_ch2); i++)
                    {
                        Channel2[0] = Channel2[0] << 1 | (Channel2[1] & 0x80) >> 7;
                        Channel2[1] =  Channel2[1] << 1;
                    }
                    Channel2[1] = Channel2[2];
                    for(int i = 1; i <= countShift_ch2; i ++)
                    {
                        Channel2[0] = (Channel2[0] << 1) | ((Channel2[1] & 0x80) >> 7);
                        Channel2[1] = Channel2[1] << 1;
                    }
                    QByteArray send2;
                    send2.append(Channel2[0]);
                    if(!ui->checkBox_Log_full->isChecked()) writeFileMSG(2, send2);
                    validitySignal(2, send2);
                    Channel2.remove(2,1);
                }
            }
            flagPackage = false;
            flagNumPackage = false;
            numBit = 0;
            flagChannel_2 = false;
        }
        else
        {
            flagPackage = false;
            flagNumPackage = false;
            numBit = 0;
        }
    }
    else
    {
        ui->label_statusPort_1->setText(" ");
        ui->label_statusPort_2->setText(" ");
        ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
        ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
        ui->label_rate_1->setText(" ");
        ui->label_rate_2->setText(" ");
        debugTextEdit(false, "Error read info");
        flagPackage = false;
        flagNumPackage = false;
        numBit = 0;
        flagChannel_1 = false;
        flagChannel_2 = false;
        return;
    }
    //Синхронизация полученного байта с эталоннами данными
    if(byteMarkerSync_CH1 == 0) return;
    if(!flagSyncCh1 && (Channel1.size() == 4))
    {
        if(byteRecieveSync_CH1 == 0)
        {
            byteRecieveSync_CH1 = Channel1[0];
            byteRecieveSync_CH1 = byteRecieveSync_CH1 << 8;
            byteRecieveSync_CH1 = byteRecieveSync_CH1 | Channel1[1];
        }
        if(byteRecieveSync_CH1 == byteMarkerSync_CH1)
        {
            flagSyncCh1 = true;
        }
        else
        {
            for(int i = 1; i <= 8; i++)
            {
                if(flagSyncCh1) break;
                byteRecieveSync_CH1 = (byteRecieveSync_CH1 << 1) | ((Channel1[2] & 0x80) >> 7);
                Channel1[2] = Channel1[2] << 1;
                if(!flagSyncCh1 && (byteRecieveSync_CH1 == byteMarkerSync_CH1))
                {
                    flagSyncCh1 = true;
                    ui->label_info_sync1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
                    debugTextEdit(true, "Synchronised Ch1");
                    countShift_ch1 = i;
                    for(int i = 1; i <= (8 - countShift_ch1); i++)
                    {
                        Channel1[0] = Channel1[0] << 1 | (Channel1[2] & 0x80) >> 7;
                        Channel1[2] =  Channel1[2] << 1;
                    }
                    Channel1[1] = Channel1[3];
                    for(int i = 1; i <= countShift_ch1; i ++)
                    {
                        Channel1[0] = (Channel1[0] << 1) | ((Channel1[1] & 0x80) >> 7);
                        Channel1[1] = Channel1[1] << 1;
                    }
                    Channel1.remove(2, 2);
                    QByteArray send1;
                    send1.append(Channel1[0]);
                    if(!ui->checkBox_Log_full->isChecked()) writeFileMSG(1, send1);
                    validitySignal(1, send1);
                }
            }
            if(!flagSyncCh1)
            {
                Channel1[2] = Channel1[3];
                Channel1.remove(3, 1);
            }
        }
    }
    if(byteMarkerSync_CH2 == 0) return;
    if(!flagSyncCh2 && Channel2.size() == 4)
    {

        if(byteRecieveSync_CH2 == 0)
        {
            byteRecieveSync_CH2 = Channel2[0];
            byteRecieveSync_CH2 = byteRecieveSync_CH2 << 8;
            byteRecieveSync_CH2 = byteRecieveSync_CH2 | Channel2[1];
        }
        if(byteRecieveSync_CH2 == byteMarkerSync_CH2)
        {
            flagSyncCh2 = true;
        }
        else
        {
            for(int i = 1; i <= 8; i++)
            {
                if(flagSyncCh2) break;
                byteRecieveSync_CH2 = (byteRecieveSync_CH2 << 1) | ((Channel2[2] & 0x80) >> 7);
                Channel2[2] = Channel2[2] << 1;
                if(!flagSyncCh2 && (byteRecieveSync_CH2 == byteMarkerSync_CH2))
                {
                    flagSyncCh2 = true;
                    ui->label_info_sync2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
                    debugTextEdit(true, "Synchronised Ch2");
                    countShift_ch2 = i;
                    for(int i = 1; i <= (8 - countShift_ch2); i++)
                    {
                        Channel2[0] = Channel2[0] << 1 | (Channel2[2] & 0x80) >> 7;
                        Channel2[2] =  Channel2[2] << 1;
                    }
                    Channel2[1] = Channel2[3];
                    for(int i = 1; i <= countShift_ch2; i ++)
                    {
                        Channel2[0] = (Channel2[0] << 1) | ((Channel2[1] & 0x80) >> 7);
                        Channel2[1] = Channel2[1] << 1;
                    }
                    Channel2.remove(2, 2);
                    QByteArray send2;
                    send2.append(Channel2[0]);
                    if(!ui->checkBox_Log_full->isChecked()) writeFileMSG(2, send2);
                    validitySignal(2, send2);
                }
            }
            if(!flagSyncCh2)
            {
                Channel2[2] = Channel2[3];
                Channel2.remove(3, 1);
            }
        }
    }
    else return;
}
//******************************************************************************
void CheckData::validitySignal(int numChannel, QByteArray byte_msg)
{
    if(byteMarkerSync_CH1 == 0) return;
    QByteArray msgControl = byteEtalon;
    if (numChannel == 1)
    {
        int cnt = 0;
        QString byteControl = QString("%1").arg((quint8)msgControl.at(countValidity_Ch1), 8, 2, QChar('0'));
        QString byteRecieve = QString("%1").arg((quint8)byte_msg.at(0), 8, 2, QChar('0'));
        for(int i = 0; i < 8; i++)
        {
            validityAll_1++;
            if (byteControl[i] == byteRecieve[i])
            {
                cnt++;
            }
        }
        validityTrue_1 = validityTrue_1 + cnt;
        if(validityAll_1 == validityTrue_1)
        {
            validity_1 = (validityAll_1 - validityTrue_1 + 1)/validityAll_1;
            if((myTime_ch1->elapsed()) >= longMsecPaint)
            {
                ui->label_nBit_CH1->setText(QString::number(validityAll_1,'g', 8));
                ui->label_nBitERR_CH1->setText(QString::number((validityAll_1 - validityTrue_1)));
                ui->label_corr_1->setText(QString::number(validity_1,'e',4));
                myTime_ch1->start();
            }
        }
        else
        {
            validity_1 = (validityAll_1 - validityTrue_1)/validityAll_1;
            if((myTime_ch1->elapsed()) >= longMsecPaint)
            {
                ui->label_nBit_CH1->setText(QString::number(validityAll_1, 'g', 8));
                ui->label_nBitERR_CH1->setText(QString::number((validityAll_1 - validityTrue_1)));
                ui->label_corr_1->setText(QString::number(validity_1,'e',4));
                myTime_ch1->start();
            }
        }
        if(countValidity_Ch1 <  (msgControl.size() - 1)) countValidity_Ch1++;
        else
        {
            countValidity_Ch1 = 0;
        }
    }
    else if(numChannel == 2)
    {
        double cnt = 0;
        QString byteControl = QString("%1").arg((int)msgControl.at(countValidity_Ch2), 8, 2, QChar('0'));
        QString byteRecieve = QString("%1").arg((int)byte_msg.at(0), 8, 2, QChar('0'));
        for(int i = 0; i < 8; i++)
        {
            validityAll_2++;
            if (byteControl[i] == byteRecieve[i])
            {
                cnt++;
            }
        }
        validityTrue_2 = validityTrue_2 + cnt;
        if(validityAll_2 == validityTrue_2)
        {
            validity_2 = (validityAll_2 - validityTrue_2 + 1)/validityAll_2;
            if((myTime_ch2->elapsed()) >= longMsecPaint)
            {
                ui->label_nBit_CH2->setText(QString::number(validityAll_2));
                ui->label_nBitERR_CH2->setText(QString::number((validityAll_2 - validityTrue_2)));
                ui->label_corr_2->setText(QString::number(validity_2,'f',4));
                myTime_ch2->start();
            }
        }
        else
        {
            validity_2 = (validityAll_2 - validityTrue_2)/validityAll_2;
            if((myTime_ch2->elapsed()) >= longMsecPaint)
            {
                ui->label_nBit_CH2->setText(QString::number(validityAll_2));
                ui->label_nBitERR_CH2->setText(QString::number((validityAll_2 - validityTrue_2)));
                ui->label_corr_2->setText(QString::number(validity_2,'f',4));
                myTime_ch2->start();
            }
        }
    }
    if(countValidity_Ch2 <  (msgControl.size() - 1)) countValidity_Ch2++;
    else
    {
        countValidity_Ch2 = 0;
        flagSyncCh2 = false;
        Channel2.clear();
    }
}
//******************************************************************************
void CheckData::openPatternFile()
{
    byteEtalon.clear();
    byteMarkerSync_CH1 = 0;
    byteMarkerSync_CH2 = 0;
    QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName.isEmpty())
    {
        debugTextEdit(false, "File isEmpty");
        return;
    }
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly))
    {
        debugTextEdit(false, "File not open");
        alarmMSG();
        return;
    }else debugTextEdit(true, "Control file load");
    while(!file.atEnd())
    {
        byteEtalon.append(file.readAll());
    }
    byteMarkerSync_CH1 = (quint16)byteEtalon[0];
    byteMarkerSync_CH1 = byteMarkerSync_CH1 << 8;
    byteMarkerSync_CH1 = byteMarkerSync_CH1 | byteEtalon[1];
    byteMarkerSync_CH2 = (quint16)byteEtalon[0];
    byteMarkerSync_CH2 = byteMarkerSync_CH2 << 8;
    byteMarkerSync_CH2 = byteMarkerSync_CH2 | byteEtalon[1];
    file.close();
}
//******************************************************************************
void CheckData::writePort(QByteArray data)
{
    port->write(data);
}
//******************************************************************************
void CheckData::writeFileMSG(int numChannel, QByteArray msg)
{
    if(ui->checkBox_Log_file->isChecked())
    {
        if(numChannel == 1)
        {
            if(file_ch_1.exists())
            {
                if(file_ch_1.isOpen())
                {
                    file_ch_1.write(msg);
                }else
                {
                    if (file_ch_1.open(QIODevice::WriteOnly | QIODevice::Append))
                    {
                        file_ch_1.write(msg);
                    }else
                    {
                        debugTextEdit(false, "File write error");
                        return;
                    }
                }
            }else
            {
                if (file_ch_1.open(QIODevice::WriteOnly))
                {
                    file_ch_1.write(msg);
                }else
                {
                    debugTextEdit(false, "File write error");
                    return;
                }
            }
        }else
        {
            if(file_ch_2.exists())
            {
                if(file_ch_2.isOpen())
                {
                    file_ch_2.write(msg);
                }else
                {
                    if (file_ch_2.open(QIODevice::WriteOnly | QIODevice::Append))
                    {
                        file_ch_2.write(msg);
                    }else
                    {
                        debugTextEdit(false, "File write error");
                        return;
                    }
                }
            }else
            {
                if (file_ch_2.open(QIODevice::WriteOnly | QIODevice::Append))
                {
                    file_ch_2.write(msg);
                }else
                {
                    debugTextEdit(false, "File write error");
                    return;
                }
            }
        }
    }
}
//******************************************************************************
void CheckData::clearFileMSG()
{
    if (file_ch_1.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file_ch_1.close();
        debugTextEdit(true, "Log file 1 cleared");
    }
    else debugTextEdit(false, "File_1 write error");
    if (file_ch_2.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file_ch_2.close();
        debugTextEdit(true, "Log file 2 cleared");
    }
    else debugTextEdit(false, "File_2 write error");
}
//******************************************************************************
void CheckData::reset_Arduino()
{
    QByteArray msg;
    msg.append(170);
    msg.append(153);

    if(port->isOpen())
    {
        writePort(msg);
        debugTextEdit(true, "Reset arduino");
    }
    else
    {
        debugTextEdit(false, "Reset err. No connect");
        return;
    }
}
//******************************************************************************
void CheckData::reset_Telementry()
{
    reset_Arduino();
    flagPackage = false;
    flagNumPackage = false;
    flagChannel_1 = false;
    flagChannel_2 = false;
    flagSyncCh1 = false;
    flagSyncCh2 = false;
    numPackage = 0;
    numBit = 0;
    countValidity_Ch1 = 2;
    countValidity_Ch2 = 2;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;
    Channel1.clear();
    Channel2.clear();
    byteRecieveSync_CH1 = 0;
    byteRecieveSync_CH2 = 0;
    countShift_ch1 = 0;
    countShift_ch2 = 0;
    save_strData = "";

    ui->label_statusPort_1->setText(" ");
    ui->label_statusPort_2->setText(" ");
    ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
    ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
    ui->label_info_sync1->setStyleSheet("QLabel {font-weight: bold; color : red; }");
    ui->label_info_sync2->setStyleSheet("QLabel {font-weight: bold; color : red; }");
    ui->label_rate_1->setText(" ");
    ui->label_rate_2->setText(" ");
    ui->label_corr_1->setText(" ");
    ui->label_corr_2->setText(" ");
    ui->label_nBit_CH1->setText(" ");
    ui->label_nBit_CH2->setText(" ");
    ui->label_nBitERR_CH1->setText(" ");
    ui->label_nBitERR_CH2->setText(" ");
}
//******************************************************************************
void CheckData::alarmMSG()
{
    if(byteEtalon.size() == 0)
    {
        QMessageBox::warning(this, "Info", "Control file not loaded!\nValidity won't work");
        return;
    }
}
//******************************************************************************
void CheckData::debugTextEdit(bool status, QString debMSG)
{
    if(status) ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> " + debMSG);
    else ui->textEdit->append("<font color = red><\\font>" + QTime::currentTime().toString("HH:mm:ss") + " -> " + debMSG);
}
//******************************************************************************
void CheckData::clear_LogDialog()
{
    ui->textEdit->clear();
}
//******************************************************************************
void CheckData::slot_StartRead()
{
    myTime_ch1->start();
    myTime_ch2->start();
    pushRead = true;
    ui->push_stop->setEnabled(true);
    ui->push_start->setEnabled(false);
}
//******************************************************************************
void CheckData::slot_StopRead()
{
    if(file_ch_1.isOpen()) file_ch_1.close();
    if(file_ch_2.isOpen()) file_ch_2.close();
    pushRead = false;
    ui->push_stop->setEnabled(false);
    ui->push_start->setEnabled(true);
    reset_Arduino();
    flagPackage = false;
    flagNumPackage = false;
    flagChannel_1 = false;
    flagChannel_2 = false;
    flagSyncCh1 = false;
    flagSyncCh2 = false;
    numPackage = 0;
    numBit = 0;
    countValidity_Ch1 = 2;
    countValidity_Ch2 = 2;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;
    Channel1.clear();
    Channel2.clear();
    byteRecieveSync_CH1 = 0;
    byteRecieveSync_CH2 = 0;
    countShift_ch1 = 0;
    countShift_ch2 = 0;
    file_ch_1.close();
    file_ch_2.close();
}
//******************************************************************************
void CheckData::refrashPort()
{
    if(num_port != (QSerialPortInfo::availablePorts().length()))
    {
        num_port = QSerialPortInfo::availablePorts().length();
        ui->comboBox->clear();
        for(int i = 0; i < num_port; i++)
        {
            ui->comboBox->addItem(QSerialPortInfo::availablePorts().at(i).portName());
        }
    }
}
