#include "CSerialPort/SerialPortBase.h"
#include "CSerialPort/ithread.hpp"
#include "CSerialPort/itimer.hpp"

CSerialPortBase::CSerialPortBase()
    : m_lastError(0)
    , m_operateMode(maix::AsynchronousOperate)
    , m_readIntervalTimeoutMS(50)
    , m_minByteReadNotify(1)
    , p_mutex(NULL)
    , p_readEvent(NULL)
    , p_timer(NULL)
{
    p_mutex = new maix::IMutex();
    p_timer = new maix::ITimer<maix::CSerialPortListener>();
}

CSerialPortBase::CSerialPortBase(const char *portName)
    : m_lastError(0)
    , m_operateMode(maix::AsynchronousOperate)
    , m_readIntervalTimeoutMS(50)
    , m_minByteReadNotify(1)
    , p_mutex(NULL)
    , p_readEvent(NULL)
    , p_timer(NULL)

{
    p_mutex = new maix::IMutex();
    p_timer = new maix::ITimer<maix::CSerialPortListener>();
}

CSerialPortBase::~CSerialPortBase()
{
    if (p_mutex)
    {
        delete p_mutex;
        p_mutex = NULL;
    }

    if (p_timer)
    {
        delete p_timer;
        p_timer = NULL;
    }
}

void CSerialPortBase::setOperateMode(maix::OperateMode operateMode)
{
    m_operateMode = operateMode;
}

unsigned int CSerialPortBase::getReadIntervalTimeout()
{
    return m_readIntervalTimeoutMS;
}

unsigned int CSerialPortBase::getMinByteReadNotify()
{
    return m_minByteReadNotify;
}

int CSerialPortBase::getLastError() const
{
    return m_lastError;
}

void CSerialPortBase::clearError() {}

int CSerialPortBase::connectReadEvent(maix::CSerialPortListener *event)
{
    if (event)
    {
        p_readEvent = event;
        return maix::NoError;
    }
    else
    {
        return maix::InvalidParameterError;
    }
}

int CSerialPortBase::disconnectReadEvent()
{
    p_readEvent = NULL;
    return maix::NoError;
}