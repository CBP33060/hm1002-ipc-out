#include <iostream>

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPort_version.h"
#include "CSerialPort/iutils.hpp"
#include "CSerialPort/itimer.hpp"

#ifdef I_OS_WIN
#include "CSerialPort/SerialPortWinBase.h"
#define CSERIALPORTBASE CSerialPortWinBase
#elif defined I_OS_UNIX
#include "CSerialPort/SerialPortUnixBase.h"
#define CSERIALPORTBASE CSerialPortUnixBase
#else
// Not support
#define CSERIALPORTBASE
#endif // I_OS_WIN

using namespace maix;

CSerialPort::CSerialPort()
    : p_serialPortBase(NULL)
{
    p_serialPortBase = new CSERIALPORTBASE();

    p_serialPortBase->setReadIntervalTimeout(0);
    p_serialPortBase->setMinByteReadNotify(1);
}

maix::CSerialPort::CSerialPort(const char *portName)
    : p_serialPortBase(NULL)
{
    p_serialPortBase = new CSERIALPORTBASE(portName);

    p_serialPortBase->setReadIntervalTimeout(0);
    p_serialPortBase->setMinByteReadNotify(1);
}

CSerialPort::~CSerialPort()
{
    if (p_serialPortBase)
    {
        delete p_serialPortBase;
        p_serialPortBase = NULL;
    }
}

void maix::CSerialPort::init(const char *portName,
                                int baudRate /*= maix::BaudRate::BaudRate9600*/,
                                maix::Parity parity /*= maix::Parity::ParityNone*/,
                                maix::DataBits dataBits /*= maix::DataBits::DataBits8*/,
                                maix::StopBits stopbits /*= maix::StopBits::StopOne*/,
                                maix::FlowControl flowControl /*= maix::FlowControl::FlowNone*/,
                                unsigned int readBufferSize /*= 4096*/)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->init(portName, baudRate, parity, dataBits, stopbits, flowControl, readBufferSize);
    }
}

void CSerialPort::setOperateMode(OperateMode operateMode)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setOperateMode(operateMode);
    }
}

bool maix::CSerialPort::open()
{
    return open(0);
}

bool maix::CSerialPort::open(int isNoBuf)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->openPort(isNoBuf);
    }
    else
    {
        return false;
    }
}

void maix::CSerialPort::close()
{
    if (p_serialPortBase)
    {
        p_serialPortBase->closePort();
    }
}

bool maix::CSerialPort::isOpen()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->isOpen();
    }
    else
    {
        return false;
    }
}

int maix::CSerialPort::connectReadEvent(maix::CSerialPortListener *event)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->connectReadEvent(event);
    }
    else
    {
        return maix::SystemError;
    }
}

int maix::CSerialPort::disconnectReadEvent()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->disconnectReadEvent();
    }
    else
    {
        return maix::SystemError;
    }
}

unsigned int maix::CSerialPort::getReadBufferUsedLen() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getReadBufferUsedLen();
    }
    else
    {
        return maix::SystemError;
    }
}

int maix::CSerialPort::readData(void *data, int size)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->readData(data, size);
    }
    else
    {
        return maix::SystemError;
    }
}

int maix::CSerialPort::readAllData(void *data)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->readAllData(data);
    }
    else
    {
        return maix::SystemError;
    }
}

int maix::CSerialPort::readLineData(void *data, int size)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->readLineData(data, size);
    }
    else
    {
        return maix::SystemError;
    }
}

int maix::CSerialPort::writeData(const void *data, int size)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->writeData(data, size);
    }
    else
    {
        return maix::SystemError;
    }
}

void maix::CSerialPort::setDebugModel(bool isDebug)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setDebugModel(isDebug);
    }
}

void maix::CSerialPort::setReadIntervalTimeout(unsigned int msecs)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setReadIntervalTimeout(msecs);
    }
}

unsigned int maix::CSerialPort::getReadIntervalTimeout()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getReadIntervalTimeout();
    }
    else
    {
        return 0;
    }
}

void CSerialPort::setMinByteReadNotify(unsigned int minByteReadNotify)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setMinByteReadNotify(minByteReadNotify);
    }
}

int maix::CSerialPort::getLastError() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getLastError();
    }
    else
    {
        // null error
        return maix::/*SerialPortError::*/ SystemError;
    }
}

void maix::CSerialPort::clearError()
{
    if (p_serialPortBase)
    {
        p_serialPortBase->clearError();
    }
}

void maix::CSerialPort::setPortName(const char *portName)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setPortName(portName);
    }
}

const char *maix::CSerialPort::getPortName() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getPortName();
    }
    else
    {
        return "";
    }
}

void maix::CSerialPort::setBaudRate(int baudRate)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setBaudRate(baudRate);
    }
}

int maix::CSerialPort::getBaudRate() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getBaudRate();
    }
    else
    {
        return maix::SystemError;
    }
}

void maix::CSerialPort::setParity(maix::Parity parity)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setParity(parity);
    }
}

maix::Parity maix::CSerialPort::getParity() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getParity();
    }
    else
    {
        // should retrun error
        return maix::/*Parity::*/ ParityNone;
    }
}

void maix::CSerialPort::setDataBits(maix::DataBits dataBits)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setDataBits(dataBits);
    }
}

maix::DataBits maix::CSerialPort::getDataBits() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getDataBits();
    }
    else
    {
        // should retrun error
        return maix::/*DataBits::*/ DataBits8;
    }
}

void maix::CSerialPort::setStopBits(maix::StopBits stopbits)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setStopBits(stopbits);
    }
}

maix::StopBits maix::CSerialPort::getStopBits() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getStopBits();
    }
    else
    {
        // should retrun error
        return maix::/*StopBits::*/ StopOne;
    }
}

void maix::CSerialPort::setFlowControl(maix::FlowControl flowControl)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setFlowControl(flowControl);
    }
}

maix::FlowControl maix::CSerialPort::getFlowControl() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getFlowControl();
    }
    else
    {
        // should retrun error
        return maix::/*FlowControl::*/ FlowNone;
    }
}

void maix::CSerialPort::setReadBufferSize(unsigned int size)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setReadBufferSize(size);
    }
}

unsigned int maix::CSerialPort::getReadBufferSize() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getReadBufferSize();
    }
    else
    {
        return maix::SystemError;
    }
}

void maix::CSerialPort::setDtr(bool set /*= true*/)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setDtr(set);
    }
}

void maix::CSerialPort::setRts(bool set /*= true*/)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setRts(set);
    }
}

const char *maix::CSerialPort::getVersion()
{
    static char version[256];
    maix::IUtils::strncpy(version, "https://github.com/maix/CSerialPort - V", 256);
    return maix::IUtils::strncat(version, CSERIALPORT_VERSION, 20);
}
