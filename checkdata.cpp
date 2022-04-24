#include "checkdata.h"
#include "ui_checkdata.h"

#include <QFileDialog>
#include <QDebug>
#include <QTime>
#include <QMessageBox>

//******************************************************************************
CheckData::CheckData(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::CheckData)
{
    ui->setupUi(this);
    setGeometry(300, 300, 480, 350);
    int num_port = QSerialPortInfo::availablePorts().length();
    for(int i = 0; i < num_port; i++)
    {
        ui->comboBox->addItem(QSerialPortInfo::availablePorts().at(i).portName());
    }
    flagPackage = false;
    flagNumPackage = false;
    flagChannel_1 = false;
    flagChannel_2 = false;
    numPackage = 0;
    numBit = 0;
    flagSyncFile_1 = false;
    flagSyncFile_2 = false;
    countValidity_Ch1 = 0;
    countValidity_Ch2 = 0;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;

    ui->progressBar_1->setValue(0);
    ui->progressBar_2->setValue(0);
    ui->comboBox_2->addItem("115200");

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
    connect(ui->push_reset_arduin, SIGNAL(clicked(bool)), this, SLOT(reset_Arduino()));
    connect(ui->push_reset_tlm, SIGNAL(clicked(bool)), this, SLOT(reset_Telementry()));
    connect(ui->push_download, SIGNAL(clicked(bool)), this, SLOT(openPatternFile()));
    connect(ui->push_clear_FileLog, SIGNAL(clicked(bool)), this, SLOT(clearFileMSG()));
    connect(ui->push_connect,SIGNAL(clicked()),this, SLOT(alarmMSG()));
    connect(ui->push_clear_log, SIGNAL(clicked(bool)), this, SLOT(clear_LogDialog()));
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
    flagSyncFile_1 = false;
    flagSyncFile_2 = false;
    Channel1.clear();
    Channel2.clear();
    countValidity_Ch1 = 0;
    countValidity_Ch2 = 0;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;

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
        ui->progressBar_1->setValue(0);
        ui->progressBar_2->setValue(0);

    }
    else return;
}
//******************************************************************************
QByteArray CheckData::readPort()
{
    QByteArray data;
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
    QString HEX;
    QString HEXmm = "0x";
    //_++
    int intData = static_cast<quint8>(data.at(0));
    for (int i = 0;i < data.size();i++)
    {
        //+++
        HEX = QString("%1").arg(intData,0,16) + tab;
        HEX = HEXmm + HEX.toUpper();
        //_++
        strData = strData+QString("%1").arg(intData)+tab;
    }
    //+++
    ui->textEdit->append(HEX);
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
            ui->progressBar_1->setValue(intData);
            ui->progressBar_2->setValue(intData);
            return;
        }
        else if(numPackage == intData - 1)
        {
            //qDebug() << intData;
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
        if(ui->progressBar_1->value() >= ui->progressBar_1->maximum()) ui->progressBar_1->reset();
        ui->progressBar_1->setValue(ui->progressBar_1->value() + 1);
        if(flagSyncFile_1)
        {
            writeFileMSG(1, data);
            validitySignal(1, data);
        }
        Channel1.append(data);
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
            if(ui->progressBar_2->value() >= ui->progressBar_2->maximum()) ui->progressBar_2->reset();
            ui->progressBar_2->setValue(ui->progressBar_2->value() + 1);
            if(flagSyncFile_2)
            {
                writeFileMSG(2, data);
                validitySignal(2, data);
            }
            Channel2.append(data);
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
    if(byteMarkerSync.size() == 0) return;
    if(!flagSyncFile_1 && Channel1.size() == 2)
    {
        if(Channel1[0] == byteMarkerSync[1])
        {
            if(Channel1[1] == byteMarkerSync[1]) flagSyncFile_1 = true;
            else Channel1.remove(0,1);
        }
        else
        {
            Channel1[0] = (Channel1[0] << 1) | (Channel1[1] & 0x80);
            //CH1Marker = QString("%1").arg((quint8)Channel1[0], 8, 2, QChar('0'));
            //qDebug() << CH1Marker;
            Channel1[0] = Channel1[0] + (Channel2[1] * 0x80);
            //CH1Marker = QString("%1").arg((quint8)byteMarkerSync[0], 8, 2, QChar('0'));
            //qDebug() << CH1Marker;
        }

        //        for(int i = 0; i < 8; i++)
        //        {
        //            validityAll_1++;
        //            if (byteControl[i] == byteRecieve[i])
        //            {
        //                cnt++;
        //            }
        //        }

        //        if(Channel1 == VPattern[0])flagSyncFile_1 = true;
        //        else Channel1.remove(0,1);
    }
    if(!flagSyncFile_2 && Channel2.size() == 2)
    {
        //        if(Channel2 == VPattern[0])flagSyncFile_2 = true;
        //        else Channel2.remove(0,1);
    }
    else return;


}
//******************************************************************************
void CheckData::validitySignal(int numChannel, QByteArray byte_msg)
{
    if(Pattern.size() == 0) return;
    QByteArray msgControl = Pattern.toLocal8Bit();
    if (numChannel == 1)
    {
        int cnt = 0;
        QString byteControl = QString("%1").arg((int)msgControl.at(countValidity_Ch1), 8, 2, QChar('0'));
        QString byteRecieve = QString("%1").arg((int)byte_msg.at(0), 8, 2, QChar('0'));
        for(int i = 0; i < 8; i++)
        {
            validityAll_1++;
            if (byteControl[i] == byteRecieve[i])
            {
                cnt++;
            }
        }
        validityTrue_1 = validityTrue_1 + cnt;
        validity_1 = validityTrue_1 / validityAll_1;
        ui->label_corr_1->setText(QString::number(validity_1));
        if(countValidity_Ch1 <  (msgControl.size() - 1)) countValidity_Ch1++;
        else
        {
            countValidity_Ch1 = 0;
            flagSyncFile_1 = false;
            Channel1.clear();
            QByteArray enter;
            enter.append("\n");
            writeFileMSG(1, enter);
            ui->progressBar_1->setValue(0);
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
        validity_2 = validityTrue_2 / validityAll_2;
        ui->label_corr_2->setText(QString::number(validity_2));
        if(countValidity_Ch2 <  (msgControl.size() - 1)) countValidity_Ch2++;
        else
        {
            countValidity_Ch2 = 0;
            flagSyncFile_2 = false;
            Channel2.clear();
            QByteArray enter;
            enter.append("\n");
            writeFileMSG(2, enter);
            ui->progressBar_2->setValue(0);
        }
    }
}
//******************************************************************************
void CheckData::openPatternFile()
{
    strEtalon.clear();
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
    byteMarkerSync.append(byteEtalon[0]);
    byteMarkerSync.append(byteEtalon[1]);

    QString CH1qMarker = QString("%1").arg((quint8)byteMarkerSync[0], 8, 2, QChar('0'));
    QString CH1qMarwker = QString("%1").arg((quint8)byteMarkerSync[1], 8, 2, QChar('0'));
    qDebug() << "__" << CH1qMarker << CH1qMarwker;
    byteMarkerSync[0] =  byteMarkerSync[0] << 1 | (byteMarkerSync[1] & 0x80) >> 7;
    byteMarkerSync[0] =  byteMarkerSync[0] << 1 | (byteMarkerSync[1] & 0x80) >> 7;
    CH1qMarker = QString("%1").arg((quint8)byteMarkerSync[0], 8, 2, QChar('0'));
    qDebug() << "__" << CH1qMarker;
    file.close();
   // byteMarkerSync[0] << 1 |
}
//******************************************************************************
void CheckData::writePort(QByteArray data)
{
    port->write(data);
}
//******************************************************************************
void CheckData::writeFileMSG(int numChannel, QByteArray msg)
{
    QString nameFile = QString("file_%1.txt").arg(QString::number(numChannel));
    QFile file_ch1(nameFile);
    if (file_ch1.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        file_ch1.write(msg);
        file_ch1.close();
    }else
    {
        debugTextEdit(false, "File write error");
        return;
    }
}
//******************************************************************************
void CheckData::clearFileMSG()
{
    QString nameFile;
    nameFile.append("file_1.txt");
    QFile file_ch1(nameFile);
    if (file_ch1.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file_ch1.close();
        debugTextEdit(true, "Log file 1 cleared");
    }
    else debugTextEdit(false, "File_1 write error");
    nameFile.clear();
    nameFile.append("file_2.txt");
    QFile file_ch2(nameFile);
    if (file_ch2.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file_ch2.close();
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
    flagPackage = false;
    flagNumPackage = false;
    flagChannel_1 = false;
    flagChannel_2 = false;
    numPackage = 0;
    numBit = 0;
    flagSyncFile_1 = false;
    flagSyncFile_2 = false;
    countValidity_Ch1 = 0;
    countValidity_Ch2 = 0;
    validity_1 = 0;
    validity_2 = 0;
    validityTrue_1 = 0;
    validityAll_1 = 0;
    validityTrue_2 = 0;
    validityAll_2 = 0;
    ui->progressBar_1->setValue(0);
    ui->progressBar_2->setValue(0);
    ui->label_statusPort_1->setText(" ");
    ui->label_statusPort_2->setText(" ");
    ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
    ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
    ui->label_rate_1->setText(" ");
    ui->label_rate_2->setText(" ");
    ui->label_corr_1->setText(" ");
    ui->label_corr_2->setText(" ");
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
