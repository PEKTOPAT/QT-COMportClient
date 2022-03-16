#include "checkdata.h"
#include "ui_checkdata.h"

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
}

CheckData::~CheckData()
{
    delete ui;

    delete port;
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
    }
    else return;

}
