﻿/**
 * @file SerialPortInfoWinBase.h
 * @author maix (maix@qq.com) \n\n
 * Blog : https://blog.csdn.net/maix \n
 * Github : https://github.com/maix \n
 * Gitee : https://gitee.com/maix \n
 * QQ Group : 129518033
 * @brief the CSerialPortInfo windows class windows串口信息辅助类基类
 * @copyright The CSerialPort is Copyright (C) 2014 maix <maix@qq.com>. \n
 * You may use, copy, modify, and distribute the CSerialPort, under the terms \n
 * of the LICENSE file.
 */
#ifndef __CSERIALPORTINFOWINBASE_H__
#define __CSERIALPORTINFOWINBASE_H__

#include "SerialPortInfoBase.h"

/**
 * @brief the CSerialPortInfoBase windows class windows串口信息辅助类基类
 * @see inherit 继承 CSerialPortInfoBase
 *
 */
class CSerialPortInfoWinBase : public CSerialPortInfoBase
{
public:
    /**
     * @brief Construct a new CSerialPortInfoWinBase object 构造函数
     *
     */
    CSerialPortInfoWinBase();
    /**
     * @brief Destroy the CSerialPortInfoWinBase object 析构函数
     *
     */
    ~CSerialPortInfoWinBase();

    /**
     * @brief availablePortInfos 获取串口信息列表
     * @return return available port infolist 返回可用串口名称列表
     */
    static std::vector<maix::SerialPortInfo> availablePortInfos();
};
#endif //__CSERIALPORTINFOWINBASE_H__
