#include "checkdata.h"
#include "ui_checkdata.h"

#include <QFileDialog>
#include <QDebug>

CheckData::CheckData(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CheckData)
{
    timer = NULL;
    timer = new QTimer();
    count = 0;
    TimeSend = 50;
    ui->setupUi(this);
    int num_port = QSerialPortInfo::availablePorts().length();
    for(int i = 0; i < num_port; i++)
    {
        ui->comboBox->addItem(QSerialPortInfo::availablePorts().at(i).portName());
    }
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
    connect(ui->push_download, SIGNAL(clicked(bool)), this, SLOT(openPatternFile()));
    connect(ui->push_send, SIGNAL(clicked(bool)), this, SLOT(sendClick()));
    connect(ui->push_stop, SIGNAL(clicked(bool)), this, SLOT(stopSendClick()));
    connect(ui->push_generate, SIGNAL(clicked(bool)), this, SLOT(createPackage()));
    connect(timer, SIGNAL(timeout()), this, SLOT(writePort()));



}

CheckData::~CheckData()
{
    delete ui;

    delete port;
    delete timer;
}
void CheckData::setRate_slot(int rate)
{
    if(rate == 0) port->setBaudRate(QSerialPort::Baud9600);
    else if (rate == 1) port->setBaudRate(QSerialPort::Baud19200);
    else if (rate == 2) port->setBaudRate(QSerialPort::Baud38400);
    else if (rate == 3) port->setBaudRate(QSerialPort::Baud57600);
    else if (rate == 4) port->setBaudRate(QSerialPort::Baud115200);
}

void CheckData::openPort()
{
    if(!port) return;
    if(port->isOpen()) port->close();
    port->setPortName(ui->comboBox->currentText());
    port->open(QIODevice::ReadWrite);
    if(port->isOpen())
    {
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + "-> Connected");
        ui->label_status->setText("Connected");
        ui->label_status->setStyleSheet("QLabel {font-weight: bold; color : green; }");
        ui->push_connect->setEnabled(false);
        ui->push_disconnect->setEnabled(true);
        ui->label_info->setText(ui->comboBox->currentText() +" @ "+ ui->comboBox_2->currentText());
    }
    else ui->textEdit->append("Port not open!");

}

void CheckData::closePort()
{
    if (! port) return;
    if(port->isOpen())
    {
        port->close();
        ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + "-> Disconnected");
        ui->label_status->setText("Disconnected");
        ui->label_status->setStyleSheet("QLabel {font-weight: bold; color : red; }");
        ui->push_connect->setEnabled(true);
        ui->push_disconnect->setEnabled(false);
        //count = 0;
    }
    else return;

}
//******************************************************************************
void CheckData::writePort()
{
    if(dataforSend.size() < 1)
    {
        debugTextEdit(false, "File not load");
        return;
    }
    else
    {
        send.append(dataforSend.at(count));
        qDebug() << "__" << send;
        port->write(send);
        send.clear();
    }
    if(count < (dataforSend.size() - 1))
    {
        count++;
    }else count = 0;

}
//******************************************************************************
void CheckData::createPackage()
{
    dataforSend.clear();
    for(int i = 1; i <= Pattern.size(); i++)
    {
        dataforSend.append(171);
        dataforSend.append(i - 1);
        dataforSend.append(230);
        QString info1 = Pattern.at(i - 1);
        dataforSend.append(info1);
        QString info2 = Pattern.at(i - 1);
        dataforSend.append(info2);
    }
    debugTextEdit(true, "Create Package");

}
//******************************************************************************
void CheckData::openPatternFile()
{
    count = 0;
    dataforSend.clear();
    QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName.isEmpty())
    {
        debugTextEdit(false, "File isEmpty");
        return;
    }
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        debugTextEdit(false, "File not open");
        return;
    }else debugTextEdit(true, "Control file load");
    QTextStream in(&file);

    while(!in.atEnd())
    {
        QString line = in.readLine();
        Pattern.append(line);
    }
    qDebug() << dataforSend;
    file.close();
}
//******************************************************************************
void CheckData::debugTextEdit(bool status, QString debMSG)
{
    if(status) ui->textEdit->append(QTime::currentTime().toString("HH:mm:ss") + " -> " + debMSG);
    else ui->textEdit->append("<font color = red><\\font>" + QTime::currentTime().toString("HH:mm:ss") + " -> " + debMSG);
}
//******************************************************************************
void CheckData::sendClick()
{
    debugTextEdit(true, "Start send");
    timer->start(TimeSend);
}
void CheckData::stopSendClick()
{
    debugTextEdit(true, "Stop send");
    timer->stop();
}


void CheckData::on_lineEdit_editingFinished()
{
    QString a = ui->lineEdit->text();
    TimeSend = a.toInt();
}
