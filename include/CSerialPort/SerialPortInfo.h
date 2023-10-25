/**
 * @file SerialPortInfo.h
 * @author maix (maix@qq.com) \n\n
 * Blog : https://blog.csdn.net/maix \n
 * Github : https://github.com/maix \n
 * Gitee : https://gitee.com/maix \n
 * QQ Group : 129518033
 * @brief the CSerialPortInfo class 串口信息辅助类
 * @copyright The CSerialPort is Copyright (C) 2014 maix <maix@qq.com>. \n
 * You may use, copy, modify, and distribute the CSerialPort, under the terms \n
 * of the LICENSE file.
 */
#ifndef __CSERIALPORTINFO_H__
#define __CSERIALPORTINFO_H__

#include <vector>

#include "SerialPort_global.h" // DLL_EXPORT

namespace maix
{
/**
 * @brief the CSerialPortInfo class 串口信息辅助类
 *
 */
class DLL_EXPORT CSerialPortInfo
{
public:
    /**
     * @brief Construct a new CSerialPortInfo object 构造函数
     *
     */
    CSerialPortInfo();
    /**
     * @brief Destroy the CSerialPortInfo object 析构函数
     *
     */
    ~CSerialPortInfo();

    /**
     * @brief availablePortInfos 获取串口信息列表
     * @return return available port infolist 返回可用串口名称列表
     */
    static std::vector<maix::SerialPortInfo> availablePortInfos();
};
} // namespace maix
#endif //__CSERIALPORTINFO_H__