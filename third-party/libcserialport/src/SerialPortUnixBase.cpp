﻿#include <unistd.h> // usleep

#include "CSerialPort/SerialPortUnixBase.h"
#include "CSerialPort/SerialPortListener.h"
#include "CSerialPort/ithread.hpp"
#include "CSerialPort/itimer.hpp"

#include <sys/prctl.h>
#include <pthread.h>

#ifdef I_OS_LINUX
// termios2 for custom baud rate at least linux kernel 2.6.32 (RHEL 6.0)

// linux/include/uapi/asm-generic/termbits.h
struct termios2
{
    tcflag_t c_iflag; /* input mode flags */
    tcflag_t c_oflag; /* output mode flags */
    tcflag_t c_cflag; /* control mode flags */
    tcflag_t c_lflag; /* local mode flags */
    cc_t c_line;      /* line discipline */
    cc_t c_cc[19];    /* control characters */
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
};

#ifndef BOTHER
#define BOTHER 0010000
#endif

// linux/include/uapi/asm-generic/ioctls.h
#ifndef TCGETS2
#define TCGETS2 _IOR('T', 0x2A, struct termios2)
#endif

#ifndef TCSETS2
#define TCSETS2 _IOW('T', 0x2B, struct termios2)
#endif

#endif

CSerialPortUnixBase::CSerialPortUnixBase()
    : fd(-1)
    , m_baudRate(maix::BaudRate9600)
    , m_parity(maix::ParityNone)
    , m_dataBits(maix::DataBits8)
    , m_stopbits(maix::StopOne)
    , m_flowControl(maix::FlowNone)
    , m_readBufferSize(4096)
    , m_isThreadRunning(false)
    , p_buffer(new maix::RingBuffer<char>(m_readBufferSize))
{
    maix::IUtils::strncpy(m_portName, "", 1);
}

CSerialPortUnixBase::CSerialPortUnixBase(const char *portName)
    : fd(-1)
    , m_baudRate(maix::BaudRate9600)
    , m_parity(maix::ParityNone)
    , m_dataBits(maix::DataBits8)
    , m_stopbits(maix::StopOne)
    , m_flowControl(maix::FlowNone)
    , m_readBufferSize(4096)
    , m_isThreadRunning(false)
    , p_buffer(new maix::RingBuffer<char>(m_readBufferSize))
{
    maix::IUtils::strncpy(m_portName, portName, 256);
}

CSerialPortUnixBase::~CSerialPortUnixBase()
{
    if (p_buffer)
    {
        delete p_buffer;
        p_buffer = NULL;
    }
}

void CSerialPortUnixBase::init(const char *portName,
                               int baudRate /*= maix::BaudRate::BaudRate9600*/,
                               maix::Parity parity /*= maix::Parity::ParityNone*/,
                               maix::DataBits dataBits /*= maix::DataBits::DataBits8*/,
                               maix::StopBits stopbits /*= maix::StopBits::StopOne*/,
                               maix::FlowControl flowControl /*= maix::FlowControl::FlowNone*/,
                               unsigned int readBufferSize /*= 4096*/)
{
    maix::IUtils::strncpy(m_portName, portName, 256); // portName;//串口 /dev/ttySn, USB /dev/ttyUSBn
    m_baudRate = baudRate;
    m_parity = parity;
    m_dataBits = dataBits;
    m_stopbits = stopbits;
    m_flowControl = flowControl;
    m_readBufferSize = readBufferSize;

    if (p_buffer)
    {
        delete p_buffer;
        p_buffer = NULL;
    }
    p_buffer = new maix::RingBuffer<char>(m_readBufferSize);
}

int CSerialPortUnixBase::uartSet(int fd, int baudRate, maix::Parity parity, maix::DataBits dataBits, maix::StopBits stopbits, maix::FlowControl flowControl)
{
    struct termios options;

    // 获取终端属性
    if (tcgetattr(fd, &options) < 0)
    {
        fprintf(stderr, "tcgetattr error");
        return -1;
    }

    // 设置输入输出波特率
    int baudRateConstant = 0;
    baudRateConstant = rate2Constant(baudRate);

    if (0 != baudRateConstant)
    {
        cfsetispeed(&options, baudRateConstant);
        cfsetospeed(&options, baudRateConstant);
    }
    else
    {
#ifdef I_OS_LINUX
        struct termios2 tio2;

        if (-1 != ioctl(fd, TCGETS2, &tio2))
        {
            tio2.c_cflag &= ~CBAUD; // remove current baud rate
            tio2.c_cflag |= BOTHER; // allow custom baud rate using int input

            tio2.c_ispeed = baudRate; // set the input baud rate
            tio2.c_ospeed = baudRate; // set the output baud rate

            if (-1 == ioctl(fd, TCSETS2, &tio2) || -1 == ioctl(fd, TCGETS2, &tio2))
            {
                fprintf(stderr, "termios2 set custom baudrate error\n");
                return -1;
            }
        }
        else
        {
            fprintf(stderr, "termios2 ioctl error\n");
            return -1;
        }
#else
        fprintf(stderr, "not support custom baudrate\n");
        return -1;
#endif
    }

    // 设置校验位
    switch (parity)
    {
        // 无奇偶校验位
        case maix::ParityNone:
            options.c_cflag &= ~PARENB; // PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~INPCK;  // INPCK：使奇偶校验起作用
            break;
        // 设置奇校验
        case maix::ParityOdd:
            options.c_cflag |= PARENB; // PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag |= PARODD; // PARODD：若设置则为奇校验,否则为偶校验
            options.c_cflag |= INPCK;  // INPCK：使奇偶校验起作用
            options.c_cflag |= ISTRIP; // ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
            break;
        // 设置偶校验
        case maix::ParityEven:
            options.c_cflag |= PARENB;  // PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~PARODD; // PARODD：若设置则为奇校验,否则为偶校验
            options.c_cflag |= INPCK;   // INPCK：使奇偶校验起作用
            options.c_cflag |= ISTRIP;  // ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
            break;
        // 设置0校验
        case maix::ParitySpace:
            options.c_cflag &= ~PARENB; // PARENB：产生奇偶位，执行奇偶校验
            options.c_cflag &= ~CSTOPB; // CSTOPB：使用两位停止位
            break;
        default:
            fprintf(stderr, "unknown parity\n");
            return -1;
    }

    // 设置数据位
    switch (dataBits)
    {
        case maix::DataBits5:
            options.c_cflag &= ~CSIZE; //屏蔽其它标志位
            options.c_cflag |= CS5;
            break;
        case maix::DataBits6:
            options.c_cflag &= ~CSIZE; //屏蔽其它标志位
            options.c_cflag |= CS6;
            break;
        case maix::DataBits7:
            options.c_cflag &= ~CSIZE; //屏蔽其它标志位
            options.c_cflag |= CS7;
            break;
        case maix::DataBits8:
            options.c_cflag &= ~CSIZE; //屏蔽其它标志位
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "unknown data bits\n");
            return -1;
    }

    // 停止位
    switch (stopbits)
    {
        case maix::StopOne:
            options.c_cflag &= ~CSTOPB; // CSTOPB：使用两位停止位
            break;
        case maix::StopOneAndHalf:
            fprintf(stderr, "POSIX does not support 1.5 stop bits\n");
            return -1;
        case maix::StopTwo:
            options.c_cflag |= CSTOPB; // CSTOPB：使用两位停止位
            break;
        default:
            fprintf(stderr, "unknown stop\n");
            return -1;
    }

    //控制模式
    options.c_cflag |= CLOCAL; //保证程序不占用串口
    options.c_cflag |= CREAD;  //保证程序可以从串口中读取数据

    // 流控制
    switch (flowControl)
    {
        case maix::FlowNone: ///< No flow control 无流控制
            options.c_cflag &= ~CRTSCTS;
            break;
        case maix::FlowHardware: ///< Hardware(RTS / CTS) flow control 硬件流控制
            options.c_cflag |= CRTSCTS;
            break;
        case maix::FlowSoftware: ///< Software(XON / XOFF) flow control 软件流控制
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
        default:
            fprintf(stderr, "unknown flow control\n");
            return -1;
    }

    // 设置输出模式为原始输出
    options.c_oflag &= ~OPOST; // OPOST：若设置则按定义的输出处理，否则所有c_oflag失效

    //设置本地模式为原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /*
     *ICANON：允许规范模式进行输入处理
     *ECHO：允许输入字符的本地回显
     *ECHOE：在接收EPASE时执行Backspace,Space,Backspace组合
     *ISIG：允许信号
     */

    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /*
     *BRKINT：如果设置了IGNBRK，BREAK键输入将被忽略
     *ICRNL：将输入的回车转化成换行（如果IGNCR未设置的情况下）(0x0d => 0x0a)
     *INPCK：允许输入奇偶校验
     *ISTRIP：去除字符的第8个比特
     *IXON：允许输出时对XON/XOFF流进行控制 (0x11 0x13)
     */

    // 设置等待时间和最小接受字符
    options.c_cc[VTIME] = 0; // 可以在select中设置
    options.c_cc[VMIN] = 1;  // 最少读取一个字符

    //如果发生数据溢出，只接受数据，但是不进行读操作
    tcflush(fd, TCIFLUSH);

    // 激活配置
    if (tcsetattr(fd, TCSANOW, &options) < 0)
    {
        perror("tcsetattr failed");
        return -1;
    }

    return 0;
}

void *CSerialPortUnixBase::commThreadMonitor(void *pParam)
{
    // Cast the void pointer passed to the thread back to
    // a pointer of CSerialPortWinBase class
    CSerialPortUnixBase *p_base = (CSerialPortUnixBase *)pParam;
    if (prctl(PR_SET_NAME, "commThreadMonitor") != 0) {
        perror("prctl");
        return NULL;
    }
    // printf("WCQ==%s==PID===%d\n",__func__,getpid());
    if (p_base)
    {
        for (; p_base->isThreadRunning();)
        {
            int readbytes = 0;

            // read前获取可读的字节数,不区分阻塞和非阻塞
            ioctl(p_base->fd, FIONREAD, &readbytes);
            if (readbytes >= p_base->getMinByteReadNotify()) //设定字符数，默认为1
            {
                char *data = NULL;
                data = new char[readbytes];
                if (data)
                {
                    if (p_base->p_buffer)
                    {
                        int len = p_base->readDataUnix(data, readbytes);
                        p_base->p_buffer->write(data, len);
#ifdef CSERIALPORT_DEBUG
                        char hexStr[201]; // 100*2 + 1
                        LOG_INFO("write buffer(usedLen %u). len: %d, hex(top100): %s", p_base->p_buffer->getUsedLen(), len,
                                 maix::IUtils::charToHexStr(hexStr, data, len > 100 ? 100 : len));
#endif

                        if (p_base->p_readEvent)
                        {
                            unsigned int readIntervalTimeoutMS = p_base->getReadIntervalTimeout();
                            if (readIntervalTimeoutMS > 0)
                            {
                                if (p_base->p_timer)
                                {
                                    if (p_base->p_timer->isRunning())
                                    {
                                        p_base->p_timer->stop();
                                    }

                                    LOG_INFO("onReadEvent. portName: %s, readLen: %u", p_base->getPortName(), p_base->p_buffer->getUsedLen());
                                    p_base->p_timer->startOnce(readIntervalTimeoutMS, p_base->p_readEvent, &maix::CSerialPortListener::onReadEvent, p_base->getPortName(),
                                                            p_base->p_buffer->getUsedLen());
                                }
                                else{
                                    printf("p_base->p_timer is NULL\n");
                                }
                            }
                            else
                            {
                                LOG_INFO("onReadEvent. portName: %s, readLen: %u", p_base->getPortName(), p_base->p_buffer->getUsedLen());
                                printf("readIntervalTimeoutMS is 0\n");
                                p_base->p_readEvent->onReadEvent(p_base->getPortName(), p_base->p_buffer->getUsedLen());
                            }
                        } else {
                            printf("p_base->p_readEvent is NULL\n");
                        }
                    } else {
                        printf("new p_base->p_buffer failed:%d\n",readbytes);    
                    }

                    delete[] data;
                    data = NULL;
                } else {
                    printf("new failed:%d\n",readbytes);
                }
            }
            else
            {
                usleep(1); // fix high cpu usage on unix
                if(p_base->p_timer->isRunning() == false && p_base->p_buffer->getUsedLen() > 0)
                {
                    p_base->p_readEvent->onReadEvent(p_base->getPortName(), p_base->p_buffer->getUsedLen());
                }
            }
        }
    }
    else
    {
        printf("point null\n");
        // point null
    }

    printf("pthread_exit\n");

    pthread_exit(NULL);
}

static int RecvBuf(int fd, char *pBuf, int iLen, int iTimeout)
{
    if(fd < 0)
    {
        return -1;
    }
    fd_set rset;
    int ret = 0;
    struct timeval t;
    size_t read_len = 0;

    int tims = iTimeout / 10;

    // printf("recv Start:\n");
    while (read_len < iLen)
    {
        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        t.tv_sec = 0;
        t.tv_usec = 10*1000;
        ret = select(fd + 1, &rset, NULL, NULL, &t);
        if (ret <= 0) {
            if (errno == EINTR) 
            {
                // 信号中断
                // printf("recv EINTR\n");
                continue;
            }

            tims--;
            // 超时（单次接收时间间隔超时）但已经接收到一些数据了
            if (ret == 0 && read_len > 0) 
            {   
                // printf("recv Done\n");
                break;
            }
            
            // 超出设定接收时间了
            if(tims <= 0)
            {
                // printf("recv Timeout\n");
                break;
            }

            if(ret < 0)
            {
                // printf("recv Select Error\n");
                return ret;
            }
        } 
        else
        {
            ret = read(fd, (char *)pBuf + read_len, iLen - read_len);
            if (ret < 0)
            {
                // printf("recv read error\n");
                return ret;
            }
            else
            {
                read_len += ret;
            }
        }
    }
    // printf("recv End\n");
    return read_len;
}

void *CSerialPortUnixBase::commThreadMonitorNoBuf(void *pParam)
{
    // Cast the void pointer passed to the thread back to
    // a pointer of CSerialPortWinBase class
    CSerialPortUnixBase *p_base = (CSerialPortUnixBase *)pParam;
    int _times = 0;
    if (prctl(PR_SET_NAME, "commThreadMonitorNoBuf") != 0) {
        perror("prctl");
        return NULL;
    }
    // printf("WCQ==%s==PID===%d\n",__func__,getpid());
    unsigned int readIntervalTimeoutMS = p_base->getReadIntervalTimeout() * 1000;
    if (p_base)
    {
        char recvBuffer[1024];
        for (; p_base->isThreadRunning();)
        {
            int _fd = p_base->fd;
            int len = RecvBuf(_fd,recvBuffer,sizeof(recvBuffer),readIntervalTimeoutMS);
            if(len > 0)
            {
                p_base->p_readEvent->onReadNoBufEvent(p_base->getPortName(), recvBuffer,len);
            }
        }
    }
    else
    {
        printf("point null\n");
        // point null
    }

    printf("pthread_exit\n");

    pthread_exit(NULL);
}

bool CSerialPortUnixBase::startThreadMonitor(int isNoBuf)
{
    bool bRet = true;

    // start read thread
    if(isNoBuf == 0)
    {
        if (0 != maix::i_thread_create(&m_monitorThread, NULL, commThreadMonitor, (void *)this))
        {
            bRet = false;

            printf("Create read thread error.");
        }
    }
    else
    {
        if (0 != maix::i_thread_create(&m_monitorThread, NULL, commThreadMonitorNoBuf, (void *)this))
        {
            bRet = false;

            printf("Create read thread error.");
        }
    }

    return bRet;
}

bool CSerialPortUnixBase::stopThreadMonitor()
{
    m_isThreadRunning = false;
    printf("stopThreadMonitor\n");
    maix::i_thread_join(m_monitorThread);

    return true;
}

bool CSerialPortUnixBase::openPort(int isNoBuf)
{
    maix::IAutoLock lock(p_mutex);

    LOG_INFO("portName: %s, baudRate: %d, dataBit: %d, parity: %d, stopBit: %d, flowControl: %d, mode: %s, readBufferSize:%u(%u), readIntervalTimeoutMS: %u, minByteReadNotify: %u",
             m_portName, m_baudRate, m_dataBits, m_parity, m_stopbits, m_flowControl, m_operateMode == maix::AsynchronousOperate ? "async" : "sync", m_readBufferSize,
             p_buffer->getBufferSize(), m_readIntervalTimeoutMS, m_minByteReadNotify);

    bool bRet = false;

    // fd = open(m_portName,O_RDWR | O_NOCTTY);//阻塞

    fd = open(m_portName, O_RDWR | O_NOCTTY | O_NDELAY); //非阻塞

    if (fd != -1)
    {
        // if(fcntl(fd,F_SETFL,FNDELAY) >= 0)//非阻塞，覆盖前面open的属性
        if (fcntl(fd, F_SETFL, 0) >= 0) // 阻塞，即使前面在open串口设备时设置的是非阻塞的，这里设为阻塞后，以此为准
        {
            // set param
            if (uartSet(fd, m_baudRate, m_parity, m_dataBits, m_stopbits, m_flowControl) == -1)
            {
                fprintf(stderr, "uart set failed\n");

                bRet = false;
                m_lastError = maix::/*SerialPortError::*/ InvalidParameterError;
            }
            else
            {
                m_isThreadRunning = true;

                bRet = startThreadMonitor(isNoBuf);
            
                if (!bRet)
                {
                    m_isThreadRunning = false;
                    m_lastError = maix::/*SerialPortError::*/ SystemError;
                }
            }
        }
        else
        {
            bRet = false;
            m_lastError = maix::/*SerialPortError::*/ SystemError;
        }
    }
    else
    {
        // Could not open the port
        char str[300];
        snprintf(str, sizeof(str), "open port error: Unable to open %s", m_portName);
        perror(str);

        bRet = false;
        m_lastError = maix::/*SerialPortError::*/ OpenError;
    }

    if (!bRet)
    {
        closePort();
    }

    return bRet;
}

void CSerialPortUnixBase::closePort()
{
    if (isOpen())
    {
        stopThreadMonitor();

        close(fd);

        fd = -1;
    }
}

bool CSerialPortUnixBase::isOpen()
{
    return fd != -1;
}

unsigned int CSerialPortUnixBase::getReadBufferUsedLen() const
{
    unsigned int usedLen = 0;

    if (m_operateMode == maix::/*OperateMode::*/ AsynchronousOperate)
    {
        usedLen = p_buffer->getUsedLen();
    }
    else
    {
        // read前获取可读的字节数,不区分阻塞和非阻塞
        ioctl(fd, FIONREAD, &usedLen);
    }

    LOG_INFO("getReadBufferUsedLen: %u", usedLen);

    return usedLen;
}

int CSerialPortUnixBase::readDataUnix(void *data, int size)
{
    maix::IAutoLock lock(p_mutex);

    int iRet = -1;

    if (isOpen())
    {
        iRet = read(fd, data, size);
    }
    else
    {
        m_lastError = maix::/*SerialPortError::*/ NotOpenError;
        iRet = -1;
    }

    return iRet;
}

int CSerialPortUnixBase::readData(void *data, int size)
{
    maix::IAutoLock lock(p_mutex);

    if (size <= 0)
    {
        return 0;
    }

    int iRet = -1;

    if (isOpen())
    {
        if (m_operateMode == maix::/*OperateMode::*/ AsynchronousOperate)
        {
            iRet = p_buffer->read((char *)data, size);
        }
        else
        {
            iRet = read(fd, data, size);
        }
    }
    else
    {
        m_lastError = maix::/*SerialPortError::*/ NotOpenError;
        iRet = -1;
    }

#ifdef CSERIALPORT_DEBUG
    char hexStr[201]; // 100*2 + 1
    LOG_INFO("read. len: %d, hex(top100): %s", iRet, maix::IUtils::charToHexStr(hexStr, (const char *)data, iRet > 100 ? 100 : iRet));
#endif

    return iRet;
}

int CSerialPortUnixBase::readAllData(void *data)
{
    return readData(data, getReadBufferUsedLen());
}

int CSerialPortUnixBase::readLineData(void *data, int size)
{
    maix::IAutoLock lock(p_mutex);

    int iRet = -1;

    if (isOpen())
    {
    }
    else
    {
        m_lastError = maix::/*SerialPortError::*/ NotOpenError;
        iRet = -1;
    }

    return iRet;
}

int CSerialPortUnixBase::writeData(const void *data, int size)
{
    maix::IAutoLock lock(p_mutex);

    int iRet = -1;

    if (isOpen())
    {
        // Write N bytes of BUF to FD.  Return the number written, or -1
        iRet = write(fd, data, size);
    }
    else
    {
        m_lastError = maix::/*SerialPortError::*/ NotOpenError;
        iRet = -1;
    }

#ifdef CSERIALPORT_DEBUG
    char hexStr[201]; // 100*2 + 1
    LOG_INFO("write. len: %d, hex(top100): %s", size, maix::IUtils::charToHexStr(hexStr, (const char *)data, size > 100 ? 100 : size));
#endif

    return iRet;
}

void CSerialPortUnixBase::setDebugModel(bool isDebug)
{
    //@todo
}

void CSerialPortUnixBase::setReadIntervalTimeout(unsigned int msecs)
{
    m_readIntervalTimeoutMS = msecs;
}

void CSerialPortUnixBase::setMinByteReadNotify(unsigned int minByteReadNotify)
{
    m_minByteReadNotify = minByteReadNotify;
}

int CSerialPortUnixBase::getLastError() const
{
    return m_lastError;
}

void CSerialPortUnixBase::clearError()
{
    m_lastError = maix::NoError;
}

void CSerialPortUnixBase::setPortName(const char *portName)
{
    maix::IUtils::strncpy(m_portName, portName, 256);
}

const char *CSerialPortUnixBase::getPortName() const
{
    return m_portName;
}

void CSerialPortUnixBase::setBaudRate(int baudRate)
{
    m_baudRate = baudRate;
}

int CSerialPortUnixBase::getBaudRate() const
{
    return m_baudRate;
}

void CSerialPortUnixBase::setParity(maix::Parity parity)
{
    m_parity = parity;
}

maix::Parity CSerialPortUnixBase::getParity() const
{
    return m_parity;
}

void CSerialPortUnixBase::setDataBits(maix::DataBits dataBits)
{
    m_dataBits = dataBits;
}

maix::DataBits CSerialPortUnixBase::getDataBits() const
{
    return m_dataBits;
}

void CSerialPortUnixBase::setStopBits(maix::StopBits stopbits)
{
    m_stopbits = stopbits;
}

maix::StopBits CSerialPortUnixBase::getStopBits() const
{
    return m_stopbits;
}

void CSerialPortUnixBase::setFlowControl(maix::FlowControl flowControl)
{
    m_flowControl = flowControl;
}

maix::FlowControl CSerialPortUnixBase::getFlowControl() const
{
    return m_flowControl;
}

void CSerialPortUnixBase::setReadBufferSize(unsigned int size)
{
    m_readBufferSize = size;
}

unsigned int CSerialPortUnixBase::getReadBufferSize() const
{
    return m_readBufferSize;
}

void CSerialPortUnixBase::setDtr(bool set /*= true*/) {}

void CSerialPortUnixBase::setRts(bool set /*= true*/) {}

bool CSerialPortUnixBase::isThreadRunning()
{
    return m_isThreadRunning;
}

int CSerialPortUnixBase::rate2Constant(int baudrate)
{
#define B(x) \
    case x:  \
        return B##x

    switch (baudrate)
    {
#ifdef B50
        B(50);
#endif
#ifdef B75
        B(75);
#endif
#ifdef B110
        B(110);
#endif
#ifdef B134
        B(134);
#endif
#ifdef B150
        B(150);
#endif
#ifdef B200
        B(200);
#endif
#ifdef B300
        B(300);
#endif
#ifdef B600
        B(600);
#endif
#ifdef B1200
        B(1200);
#endif
#ifdef B1800
        B(1800);
#endif
#ifdef B2400
        B(2400);
#endif
#ifdef B4800
        B(4800);
#endif
#ifdef B9600
        B(9600);
#endif
#ifdef B19200
        B(19200);
#endif
#ifdef B38400
        B(38400);
#endif
#ifdef B57600
        B(57600);
#endif
#ifdef B115200
        B(115200);
#endif
#ifdef B230400
        B(230400);
#endif
#ifdef B460800
        B(460800);
#endif
#ifdef B500000
        B(500000);
#endif
#ifdef B576000
        B(576000);
#endif
#ifdef B921600
        B(921600);
#endif
#ifdef B1000000
        B(1000000);
#endif
#ifdef B1152000
        B(1152000);
#endif
#ifdef B1500000
        B(1500000);
#endif
#ifdef B2000000
        B(2000000);
#endif
#ifdef B2500000
        B(2500000);
#endif
#ifdef B3000000
        B(3000000);
#endif
#ifdef B3500000
        B(3500000);
#endif
#ifdef B4000000
        B(4000000);
#endif
        default:
            return 0;
    }

#undef B
}