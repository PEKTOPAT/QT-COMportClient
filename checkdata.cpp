#include "checkdata.h"
#include "ui_checkdata.h"

#include <QFileDialog>
#include <QDebug>
#include <QTime>


//******************************************************************************
CheckData::CheckData(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CheckData)
{
    ui->setupUi(this);
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
    numByte = 0;
    flagSyncFile_1 = false;
    flagSyncFile_2 = false;

    port = new QSerialPort(this);
    ui->comboBox_2->addItem("9600");
    ui->comboBox_2->addItem("19200");
    ui->comboBox_2->addItem("38400");
    ui->comboBox_2->addItem("57600");
    ui->comboBox_2->addItem("115200");

    port->setDataBits(QSerialPort::Data8);
    port->setFlowControl(QSerialPort::NoFlowControl);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);

    //connecting
    connect(ui->push_connect,SIGNAL(clicked()),this, SLOT(openPort()));
    connect(ui->push_disconnect,SIGNAL(clicked()),this, SLOT(closePort()));
    connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(setRate_slot(int)));
    connect(port, SIGNAL(readyRead()), this, SLOT(readPort()));
    connect(ui->push_reset, SIGNAL(clicked(bool)), this, SLOT(reset_Arduino()));
    connect(ui->push_download, SIGNAL(clicked(bool)), this, SLOT(openPatternFile()));
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
    numByte = 0;
    flagSyncFile_1 = false;
    flagSyncFile_2 = false;
    Channel1.clear();
    Channel2.clear();
    VPattern.clear();

    port->setPortName(ui->comboBox->currentText());
    port->open(QIODevice::ReadWrite);
    if(port->isOpen())
    {
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> Connected");
        ui->label_status->setText("Connected");
        ui->label_status->setStyleSheet("QLabel {font-weight: bold; color : green; }");
        ui->push_connect->setEnabled(false);
        ui->push_disconnect->setEnabled(true);
        ui->label_info->setText(ui->comboBox->currentText() +" @ "+ ui->comboBox_2->currentText());
    }
    else ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> Port not open!");

}
//******************************************************************************
void CheckData::closePort()
{
    if (! port) return;
    if(port->isOpen())
    {

        port->close();
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> Disconnected");
        ui->label_status->setText("Disconnected");
        ui->label_status->setStyleSheet("QLabel {font-weight: bold; color : red; }");
        ui->push_connect->setEnabled(true);
        ui->push_disconnect->setEnabled(false);
    }
    else return;
}
//******************************************************************************
QByteArray CheckData::readPort()
{
    QByteArray data;
    if (port->bytesAvailable() == 0) return data;
    data = port->readAll();
    parsingPackage(data);
    return data;
}

void CheckData::parsingPackage(QByteArray data)
{
    if(data.size() == 0) return;
    const QString tab = " ";
    QString strData;
    int intData;
    qDebug() << "DEBUG  " << numByte;
    intData = static_cast<quint8>(data.at(0));
    for (int i = 0;i < data.size();i++)
    {
        strData = strData+QString("%1").arg(intData)+tab;
    }
    strData.resize(strData.length() - 1);
    qDebug() <<"DEBUG recieve msg"<< strData;
    if(!flagPackage && numByte == 0)
    {
        if(strData == "171")
        {
            flagPackage = true;
            numByte = 1;
            qDebug() << "Обнаружена посылка!";

        }else
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_statusPort_2->setText(" ");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_rate_1->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->textEdit->append("Error detect marker!");
        }
    }
    else if(numPackage == 0 && numByte == 1)
    {
        numPackage = intData;
        flagNumPackage = true;
        numByte = 2;
        qDebug() << "Посылка номер" << intData;
    }
    else if(!flagNumPackage && numPackage == intData - 1 && numByte == 1)
    {
        numPackage++;
        flagNumPackage = true;
        numByte = 2;
        qDebug() << "Посылка номер " << numPackage;
    }
    else if(!flagChannel_1 && !flagChannel_2 && numByte == 2)
    {
        if(strData == "161")
        {
            ui->label_statusPort_2->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            flagChannel_1 = true;
            numByte = 3;
        }
        else if (strData == "162")
        {
            ui->label_statusPort_2->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            flagChannel_1 = true;
            numByte = 3;
        }
        else if (strData == "163")
        {
            ui->label_statusPort_2->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("9,6 KB/s");
            flagChannel_1 = true;
            numByte = 3;

        }
        else if (strData == "196")
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_rate_1->setText(" ");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_2 = true;
            numByte = 3;

        }
        else if (strData == "200")
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_rate_1->setText(" ");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "204")
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_rate_1->setText(" ");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_2->setText("9,6 KB/s");
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "229")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "230")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "231")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("2,4 KB/s");
            ui->label_rate_2->setText("9,6 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "233")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "234")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "235")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("4,8 KB/s");
            ui->label_rate_2->setText("9,6 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "237")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("9,6 KB/s");
            ui->label_rate_2->setText("2,4 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "238")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("9,6 KB/s");
            ui->label_rate_2->setText("4,8 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "239")
        {
            ui->label_statusPort_1->setText("Up");
            ui->label_statusPort_2->setText("Up");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : green; }");
            ui->label_rate_1->setText("9,6 KB/s");
            ui->label_rate_2->setText("9,6 KB/s");
            flagChannel_1 = true;
            flagChannel_2 = true;
            numByte = 3;
        }
        else if (strData == "128")
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_statusPort_2->setText(" ");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_rate_1->setText(" ");
            ui->label_rate_2->setText(" ");
            numByte = 3;
        }else
        {
            ui->label_statusPort_1->setText(" ");
            ui->label_statusPort_2->setText(" ");
            ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
            ui->label_rate_1->setText(" ");
            ui->label_rate_2->setText(" ");
            ui->textEdit->append("Error synchronization!");
        }
    }
    //to ASCII
    else if (flagChannel_1 && numByte == 3)
    {
        strData.clear();
        for (int i = 0; i < data.size();i++)
            switch (data.at(i))
            {
            case 0:
                strData += "\'0\'";
                break;
            default:
                strData += data[i];
            }
        Channel1.append(strData);
        qDebug() << "Запись с первого канала" << Channel1;
        QFile file_ch1("file_ch1.txt");
        if (file_ch1.open(QIODevice::WriteOnly | QIODevice::Append)) {
            file_ch1.write(data);
            file_ch1.close();
        }else {qDebug() << "fff"; return;}
        flagChannel_1 = false;
        numByte = 4;
    }
    else if (!flagChannel_1 && numByte == 3)
    {
        numByte = 4;
    }
    //to ASCII
    else if (flagChannel_2 &&  numByte == 4)
    {
        strData.clear();
        for (int i = 0; i < data.size();i++)
            switch (data.at(i))
            {
            case 0:
                strData += "\'0\'";
                break;
            default:
                strData += data[i];
            }
        Channel2.append(strData);
        qDebug() << "Запись со второго канала" << Channel2;
        flagPackage = false;
        flagNumPackage = false;
        flagChannel_1 = false;
        flagChannel_2 = false;
        numByte = 0;
    }
    else
    {
        ui->label_statusPort_1->setText(" ");
        ui->label_statusPort_2->setText(" ");
        ui->label_statusPort_1->setStyleSheet("QLabel {font-weight: bold; color : black; }");
        ui->label_statusPort_2->setStyleSheet("QLabel {font-weight: bold; color : black; }");
        ui->label_rate_1->setText(" ");
        ui->label_rate_2->setText(" ");
        ui->textEdit->append("Error");
    }
    if(!flagSyncFile_1)
    {
        if(Channel1.size() == 2 && Channel1 == "ma")
        {
            flagSyncFile_1 = true;
            if(VPattern.size() != 0) validitySignal(Channel1, "");
            else ui->textEdit->append("Error, not download file!");
        }
    }
}
//******************************************************************************
void CheckData::writePort(QByteArray data)
{
    port->write(data);
}
//******************************************************************************
void CheckData::openPatternFile()
{
    VPattern.clear();
    QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName.isEmpty())
    {
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> File isEmpty");
        return;
    }
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> File not open");
        return;
    }else ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> File load");
    QTextStream in(&file);
    QString line = in.readLine();
    VPattern.append(line);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        VPattern.append(line);
    }
    file.close();
}
//******************************************************************************
void CheckData::validitySignal(QString syncInfo,  QString receive_Byte)
{
    qDebug() << "Ура";
}

//******************************************************************************
void CheckData::reset_Arduino()
{
    QByteArray msg;
    msg.append(170);
    if(port->isOpen())
    {
        writePort(msg);
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> reset");
    }
    else return;
}
//******************************************************************************
