/**
 * @file SerialPortUnixBase.h
 * @author maix (maix@qq.com) \n\n
 * Blog : https://blog.csdn.net/maix \n
 * Github : https://github.com/maix \n
 * Gitee : https://gitee.com/maix \n
 * QQ Group : 129518033
 * @brief the CSerialPort unix Base class unix串口基类
 * @copyright The CSerialPort is Copyright (C) 2014 maix <maix@qq.com>. \n
 * You may use, copy, modify, and distribute the CSerialPort, under the terms \n
 * of the LICENSE file.
 */
#ifndef __CSERIALPORTUNIXBASE_H__
#define __CSERIALPORTUNIXBASE_H__

#include <stdio.h>     // Standard input/output definitions
#include <string.h>    // String function definitions
#include <sys/ioctl.h> // ioctl
#include <termios.h>   // POSIX terminal control definitions
#include <fcntl.h>     // File control definitions
#include <unistd.h>    // UNIX standard function definitions
#include <errno.h>     // Error number definitions

#include "ithread.hpp"
#include "ibuffer.hpp"
#include "SerialPortBase.h"

// Serial Programming Guide for POSIX Operating Systems
// https://digilander.libero.it/robang/rubrica/serial.htm

/**
 * @brief the CSerialPort unix Base class unix串口基类
 * @see inherit 继承 CSerialPortBase
 *
 */
class CSerialPortUnixBase : public CSerialPortBase
{
public:
    /**
     * @brief Construct a new CSerialPortUnixBase object 构造函数
     *
     */
    CSerialPortUnixBase();
    /**
     * @brief Construct a new CSerialPortUnixBase object 通过串口名称构造函数
     *
     * @param portName [in] the port name 串口名称 Windows:COM1 Linux:/dev/ttyS0
     */
    CSerialPortUnixBase(const char *portName);
    /**
     * @brief Destroy the CSerialPortUnixBase object 析构函数
     *
     */
    virtual ~CSerialPortUnixBase();

    /**
     * @brief init 初始化函数
     *
     * @param portName [in] the port name串口名称 Windows:COM1 Linux:/dev/ttyS0
     * @param baudRate [in] the baudRate 波特率
     * @param parity [in] the parity 校验位
     * @param dataBits [in] the dataBits 数据位
     * @param stopbits [in] the stopbits 停止位
     * @param flowControl [in] flowControl type 流控制
     * @param readBufferSize [in] the read buffer size 读取缓冲区大小
     */
    virtual void init(const char *portName,
                      int baudRate = maix::BaudRate9600,
                      maix::Parity parity = maix::ParityNone,
                      maix::DataBits dataBits = maix::DataBits8,
                      maix::StopBits stopbits = maix::StopOne,
                      maix::FlowControl flowControl = maix::FlowNone,
                      unsigned int readBufferSize = 4096);

    /**
     * @brief open serial port 打开串口
     *
     * @return
     * @retval true open success 打开成功
     * @retval false open failed 打开失败
     */
    virtual bool openPort(int isNoBuf);
    /**
     * @brief close 关闭串口
     *
     */
    virtual void closePort();

    /**
     * @brief if serial port is open success 串口是否打开成功
     *
     * @return
     * @retval true serial port open success 串口打开成功
     * @retval false serial port open failed 串口打开失败
     */
    virtual bool isOpen();

    /**
     * @brief get used length of buffer 获取读取缓冲区已使用大小
     *
     * @return return used length of buffer 返回读取缓冲区已使用大小
     */
    virtual unsigned int getReadBufferUsedLen() const;

    /**
     * @brief read specified length data 读取指定长度数据
     *
     * @param data [out] read data result 读取结果
     * @param size [in] read length 读取长度
     * @return return number Of bytes read 返回读取字节数
     * @retval -1 read error 读取错误
     * @retval [other] return number Of bytes read 返回读取字节数
     */
    virtual int readData(void *data, int size);
    /**
     * @brief read all data 读取所有数据
     *
     * @param data [out] read data result 读取结果
     * @return return number Of bytes read 返回读取字节数
     * @retval -1 read error 读取错误
     * @retval [other] return number Of bytes read 返回读取字节数
     */
    virtual int readAllData(void *data);
    /**
     * @brief read line data 读取一行字符串
     * @todo Not implemented 未实现
     *
     * @param data
     * @param size
     * @return int
     */
    virtual int readLineData(void *data, int size);
    /**
     * @brief write specified lenfth data 写入指定长度数据
     *
     * @param data [in] write data 待写入数据
     * @param size [in] wtite length 写入长度
     * @return return number Of bytes write 返回写入字节数
     * @retval -1 read error 写入错误
     * @retval [other] return number Of bytes write 返回写入字节数
     */
    virtual int writeData(const void *data, int size);

    /**
     * @brief Set Debug Model 设置调试模式
     * @details output serial port read and write details info 输出串口读写的详细信息
     * @todo  Not implemented 未实现
     *
     * @param isDebug true if enable true为启用
     */
    virtual void setDebugModel(bool isDebug);

    /**
     * @brief Set Read Interval Timeout millisecond
     * @details use timer import effectiveness 使用定时器提高效率
     *
     * @param msecs read time timeout millisecond 读取间隔时间，单位：毫秒
     */
    virtual void setReadIntervalTimeout(unsigned int msecs);

    /**
     * @brief setMinByteReadNotify set minimum byte of read notify 设置读取通知触发最小字节数
     * @param minByteReadNotify minimum byte of read notify 读取通知触发最小字节数
     */
    virtual void setMinByteReadNotify(unsigned int minByteReadNotify = 2);

    /**
     * @brief Get the Last Error object 获取最后的错误代码
     *
     * @return return last error code, refrence {@link maix::SerialPortError} 错误代码
     */
    virtual int getLastError() const;
    /**
     * @brief clear error 清除错误信息
     *
     */
    virtual void clearError();

    /**
     * @brief Set the Port Name object 设置串口名称
     *
     * @param portName [in] the port name 串口名称 Windows:COM1 Linux:/dev/ttyS0
     */
    virtual void setPortName(const char *portName);
    /**
     * @brief Get the Port Name object 获取串口名称
     *
     * @return return port name 返回串口名称
     */
    virtual const char *getPortName() const;

    /**
     * @brief Set the Baud Rate object 设置波特率
     *
     * @param baudRate [in] the baudRate 波特率
     */
    virtual void setBaudRate(int baudRate);
    /**
     * @brief Get the Baud Rate object 获取波特率
     *
     * @return return baudRate 返回波特率
     */
    virtual int getBaudRate() const;

    /**
     * @brief Set the Parity object 设置校验位
     *
     * @param parity [in] the parity 校验位 {@link maix::Parity}
     */
    virtual void setParity(maix::Parity parity);
    /**
     * @brief Get the Parity object 获取校验位
     *
     * @return return parity 返回校验位 {@link maix::Parity}
     */
    virtual maix::Parity getParity() const;
    /**
     * @brief Set the Data Bits object 设置数据位
     *
     * @param dataBits [in] the dataBits 数据位 {@link maix::DataBits}
     */
    virtual void setDataBits(maix::DataBits dataBits);
    /**
     * @brief Get the Data Bits object 获取数据位
     *
     * @return return dataBits 返回数据位 {@link maix::DataBits}
     */
    virtual maix::DataBits getDataBits() const;

    /**
     * @brief Set the Stop Bits object 设置停止位
     *
     * @param stopbits [in] the stopbits 停止位 {@link maix::StopBits}
     */
    virtual void setStopBits(maix::StopBits stopbits);
    /**
     * @brief Get the Stop Bits object 获取停止位
     *
     * @return return stopbits 返回停止位 {@link maix::StopBits}
     */
    virtual maix::StopBits getStopBits() const;

    /**
     * @brief Set the Flow Control object 设置流控制
     * @todo Not implemented 未实现
     *
     * @param flowControl [in]
     */
    virtual void setFlowControl(maix::FlowControl flowControl);
    /**
     * @brief Get the Flow Control object 获取流控制
     * @todo Not implemented 未实现
     *
     * @return maix::FlowControl
     */
    virtual maix::FlowControl getFlowControl() const;

    /**
     * @brief Set the Read Buffer Size object 设置读取缓冲区大小
     *
     * @param size [in] read buffer size 读取缓冲区大小
     */
    virtual void setReadBufferSize(unsigned int size);
    /**
     * @brief Get the Read Buffer Size object 获取读取缓冲区大小
     *
     * @return return read buffer size 返回读取缓冲区大小
     */
    virtual unsigned int getReadBufferSize() const;

    /**
     * @brief Set the Dtr object 设置DTR
     * @todo Not implemented 未实现
     *
     * @param set [in]
     */
    virtual void setDtr(bool set = true);
    /**
     * @brief Set the Rts object 设置RTS
     * @todo Not implemented 未实现
     *
     * @param set [in]
     */
    virtual void setRts(bool set = true);

public:
    /**
     * @brief isThreadRunning 是否启动多线程
     * @return
     * @retval true thread running 多线程已启动
     * @retval false thread not running 多线程未启动
     */
    bool isThreadRunning();

private:
    /**
     * @brief rate2Constant baudrate to constant 波特率转为unix常量
     * @param baudrate 波特率
     * @return constant unix常量
     */
    int rate2Constant(int baudrate);

    /**
     * @brief uartSet
     * @param fd [in] file discriptor 文件描述符
     * @param baudRate [in] the baudRate 波特率
     * @param parity [in] the parity 校验位
     * @param dataBits [in] the dataBits 数据位
     * @param stopbits [in] the stopbits 停止位
     * @param flowControl [in] flowControl type 流控制
     * @return 0 success -1 error
     */
    int uartSet(int fd,
                int baudRate = maix::BaudRate9600,
                maix::Parity parity = maix::ParityNone,
                maix::DataBits dataBits = maix::DataBits8,
                maix::StopBits stopbits = maix::StopOne,
                maix::FlowControl flowControl = maix::FlowNone);

    /**
     * @brief thread monitor 多线程监视器
     *
     */
    static void *commThreadMonitor(void *pParam);
    static void *commThreadMonitorNoBuf(void *pParam);

    /**
     * @brief start thread monitor 启动多线程监视器
     *
     * @return
     * @retval true start success 启动成功
     * @retval false start failed 启动失败
     */
    bool startThreadMonitor(int isNobuf);
    /**
     * @brief stop thread monitor 停止多线程监视器
     *
     * @return
     * @retval true stop success 停止成功
     * @retval false stop failed 停止失败
     */
    bool stopThreadMonitor();

    /**
     * @brief read specified length data 读取指定长度数据
     *
     * @param data [out] read data result 读取结果
     * @param size [in] read length 读取长度
     * @return return number Of bytes read 返回读取字节数
     * @retval -1 read error 读取错误
     * @retval [other] return number Of bytes read 返回读取字节数
     */
    virtual int readDataUnix(void *data, int size);

private:
    char m_portName[256];
    int m_baudRate;
    maix::Parity m_parity;
    maix::DataBits m_dataBits;
    maix::StopBits m_stopbits;
    maix::FlowControl m_flowControl;
    unsigned int m_readBufferSize;

public:
    int fd; /* File descriptor for the port */

private:
    maix::i_thread_t m_monitorThread; /**< read thread */

    bool m_isThreadRunning;

    maix::RingBuffer<char> *p_buffer; ///< receive buffer
};
#endif //__CSERIALPORTUNIXBASE_H__
