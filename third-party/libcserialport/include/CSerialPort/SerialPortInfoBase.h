﻿/**
 * @file SerialPortInfoBase.h
 * @author maix (maix@qq.com) \n\n
 * Blog : https://blog.csdn.net/maix \n
 * Github : https://github.com/maix \n
 * Gitee : https://gitee.com/maix \n
 * QQ Group : 129518033
 * @brief the CSerialPortInfo Base class 串口信息辅助类基类
 * @copyright The CSerialPort is Copyright (C) 2014 maix <maix@qq.com>. \n
 * You may use, copy, modify, and distribute the CSerialPort, under the terms \n
 * of the LICENSE file.
 */
#ifndef __CSERIALPORTINFOBASE_H__
#define __CSERIALPORTINFOBASE_H__

#include <vector>

namespace maix
{
struct SerialPortInfo; // Forward Declaration
}

/**
 * @brief the CSerialPortInfo Base class 串口信息辅助类基类
 *
 */
class CSerialPortInfoBase
{
public:
    /**
     * @brief Construct a new CSerialPortInfoBase object 构造函数
     *
     */
    CSerialPortInfoBase();
    /**
     * @brief Destroy the CSerialPortInfoBase object 析构函数
     *
     */
    ~CSerialPortInfoBase();

    /**
     * @brief availablePortInfos 获取串口信息列表
     * @return return available port infolist 返回可用串口名称列表
     */
    static std::vector<maix::SerialPortInfo> availablePortInfos(void);
};
#endif //__CSERIALPORTINFOBASE_H__