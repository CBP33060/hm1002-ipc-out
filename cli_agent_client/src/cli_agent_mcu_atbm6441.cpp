#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "cli/pch.h"
#include "cli/common.h"
#include "cli_agent_mcu.hpp"
#include "cli_agent_mcu_atbm6441.hpp"
#include "cli_agent_common.hpp"

#include "fw_env_para.h"

#include "iot.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "mx_proto_common.h"
#include "mx_proto_decode.h"
#include "mx_proto_encode.h"
#include "mx_comm_adapter.h"
#include "mx_proto_system.h"
#include "mx_proto_factory.h"

#include "cJSON.h"
#include "common.h"

static int32_t commUartSendCb(void *data, uint16_t len)
{   
    maix::CCliAgentMcu *CliAgentMcu = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);
    maix::CCliAgentMcuIoDevice *CliAgentMcuIo = CliAgentMcu->GetIO();

	CliAgentMcuIo->SendBuf((unsigned char *)data, len);

	return 0;
}

static int32_t commUartRecvHandleCb(IotPacket *pkt)
{
	maix::CCliAgentMcu *CliAgentMcu = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);

    CliAgentMcu->RecvHandler((void *)pkt);

	return 0;
}

namespace maix {
    CCliAgentMcuAtbm6441::CCliAgentMcuAtbm6441()
    {
        m_iRecvBufLen = 1024;
        m_pRecvBuf = new unsigned char[m_iRecvBufLen];

        m_bRunning = false;
        m_iCapacity = 0;
		m_iCurrent = 0;
		m_iVoltage = 0;
        m_iOOBFlag = 0;
		m_iTemperature = 0;
        m_iUpgradeFlag = 0;
        comm_proto_init_wrapper(&commUartSendCb);
		mx_proto_packet_rcv_cb_register(&commUartRecvHandleCb);
    }

    CCliAgentMcuAtbm6441::~CCliAgentMcuAtbm6441()
    {
        if(m_pRecvBuf != NULL)
        {
            delete m_pRecvBuf;
        }
    }

    int CCliAgentMcuAtbm6441::SendBuf(unsigned char *pBuf, int iLen)
    {
        CCliAgentMcuIoDevice *CliAgentMcuIo = GetIO();
        if(CliAgentMcuIo == NULL)
        {
            return -1;
        }

        return CliAgentMcuIo->SendBuf(pBuf, iLen);
    }

	int CCliAgentMcuAtbm6441::RecvBuf(unsigned char *pBuf, int iLen, int iTimeout)
    {
        CCliAgentMcuIoDevice *CliAgentMcuIo = GetIO();
        if(CliAgentMcuIo == NULL)
        {
            return -1;
        }

        return CliAgentMcuIo->RecvBuf(pBuf, iLen, iTimeout);
    }
    
    int CCliAgentMcuAtbm6441::SendToAtbmd(std::string strCommand)
    {
        char command[16000];
        struct sockaddr_un ser_un;
        int tmp_argc = 1;
        int socket_fd = 0;
        int ret = 0;
        int i = 0;

        memset(command, 0, sizeof(command));
        strncpy(command,strCommand.c_str(),strlen(strCommand.c_str()));

        socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socket_fd <= 0)
        {
            cli::OutputDevice::GetStdErr() << "open socket err" << cli::endl;
            return -1;
        }

        memset(&ser_un, 0, sizeof(ser_un));  
        ser_un.sun_family = AF_UNIX;  
        strcpy(ser_un.sun_path, "/usr/wifi/server_socket");

        ret = connect(socket_fd, (struct sockaddr *)&ser_un, sizeof(struct sockaddr_un));  
        if(ret < 0)  
        {  
            cli::OutputDevice::GetStdErr() << "connect err" << cli::endl;
            return -2; 
        }

        write(socket_fd, command, strlen(command)+1);
        read(socket_fd, command, sizeof(command));
        if (strcmp(command, "OK"))
        {
            cli::OutputDevice::GetStdErr() << "send cmd err" << cli::endl;
            return -3;
        }

        return 0;
    }

    int CCliAgentMcuAtbm6441::SendFactoryCmd(std::string strCommand)
    {
        CmdInfo_Request info;
        int ret = 0;

        info.data = (pb_bytes_array_t *)malloc(sizeof(pb_bytes_array_t) + sizeof(pb_byte_t)*(strCommand.length() + 1));
        if(info.data == NULL)
        {
            //cli::OutputDevice::GetStdErr() << "SendFactoryCmd malloc failed" << cli::endl;
            ret = -1;
        }

        // cli::OutputDevice::GetStdErr() << "SendFactoryCmd length:" << strCommand.length() << " " << strCommand << cli::endl;

		memcpy(info.data->bytes, strCommand.c_str(), strCommand.length());
        info.data->bytes[strCommand.length()] = 0;
		info.data->size = strCommand.length() + 1;

        factory_cmd_info_request(&info);

        //cli::OutputDevice::GetStdErr() << "SendFactoryCmd end" << cli::endl;
        free(info.data);

        return ret;
    }

    /* * * * * * * * * * * * * * * * * * */

    int CCliAgentMcuAtbm6441::SetPSN(std::string strSN)
    {
        return maix::setFWParaConfig("factory","psn",strSN.c_str(),1);
    }

    std::string CCliAgentMcuAtbm6441::GetPSN()
    {
        char * str = maix::getFWParaConfig("factory","psn");
        
        if(str == NULL)
        {
            return "";
        }

        return str;
    }

    int CCliAgentMcuAtbm6441::SetSN(std::string strSN)
    {
        return maix::setFWParaConfig("factory","sn",strSN.c_str(),1);
    }

    std::string CCliAgentMcuAtbm6441::GetSN()
    {
        char * str = maix::getFWParaConfig("factory","sn");
        
        if(str == NULL)
        {
            return "";
        }

        return str;
    }

    int CCliAgentMcuAtbm6441::SetDID(std::string strDID)
    {
        std::string strCmd = "AT+SYS_SET_DID " + strDID;

        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd(strCmd);
        RecvMcuMsg(3000);

        cli::OutputDevice::GetStdErr() << "AT+SYS_SET_DID:" << m_strFactoryResponse << cli::endl;

        setFWParaConfig("factory","did",strDID.c_str(),1);
        return setDID(strDID.c_str());
    }

    std::string CCliAgentMcuAtbm6441::GetDID()
    {
        return getDID();
    }

    int CCliAgentMcuAtbm6441::SetPID(std::string strPID)
    {
        return setFWParaConfig("factory","pid",strPID.c_str(),1);
    }

    std::string CCliAgentMcuAtbm6441::GetPID()
    {
        return maix::getFWParaConfig("factory","pid");
    }

    std::string FormatMacAddress(const std::string& MacAddress) 
    {
        std::string FormattedMacAddress;

        for (size_t i = 0; i < MacAddress.length(); i += 2) {
            FormattedMacAddress += MacAddress.substr(i, 2);
            if (i + 2 < MacAddress.length()) {
                FormattedMacAddress += ":";
            }
        }

        return FormattedMacAddress;
    }

    int CCliAgentMcuAtbm6441::SetMAC(std::string strMAC)
    {
        std::string strMACColon = FormatMacAddress(strMAC);
        std::string strCmd = "AT+WIFI_AP_MAC ADDR " + strMACColon;

        setFWParaConfig("factory","mac",strMACColon.c_str(),1);
        setMAC(strMAC.c_str());
        return WifiCmd(strCmd);
    }
    
    std::string CCliAgentMcuAtbm6441::GetMAC()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+WSMAC");
        RecvMcuMsg(3000);

        //cli::OutputDevice::GetStdErr() << "GetMAC:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

    int CCliAgentMcuAtbm6441::SetKey(std::string strKey)
    {
        setFWParaConfig("factory","key",strKey.c_str(),1);
        return setPSK(strKey.c_str());
    }

    std::string CCliAgentMcuAtbm6441::GetKey()
    {
        return getPSK();
    }

    std::string CCliAgentMcuAtbm6441::GetOOB()
    {
        return maix::getFWParaConfig("factory","oob");
    }

    std::string CCliAgentMcuAtbm6441::GenOOB()
    {
        int ret = -1;
        int connetErrno = 0;
        char buf[256];
        std::string oob;
        if(m_iOOBFlag == 0)
        {
            system("/system/mi_ot/miio_client_helper_nomqtt.sh.mijia &");
            m_iOOBFlag = 1;
        }
        std::string strCommond = "{\"method\":\"local.query_qr_code\",\"params\":\"\",\"id\":123456}";
        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(54322);
        server.sin_addr.s_addr = inet_addr("127.0.0.1");

        int sockClient = socket(AF_INET, SOCK_STREAM, 0);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sockClient, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
        setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

        ret = connect(sockClient, (struct sockaddr*)&server, sizeof(struct sockaddr));
        if (ret)
        {
            connetErrno = errno;
            printf("connect error: %s\n", strerror(connetErrno));
            return oob;
        }
        else
        {
            printf("Connect successful!\n");
        }

        ret = send(sockClient, strCommond.c_str(), strCommond.length(), 0);
        if (ret == -1)
        {
            connetErrno = errno;
            printf("send error: %s\n", strerror(connetErrno));
            return oob;
        }

        ret = recv(sockClient, buf, sizeof(buf), 0);
        if (ret == -1)
        {
            connetErrno = errno;
            printf("recv error: %s\n", strerror(connetErrno));
            return oob;
        }

        cJSON *root = cJSON_Parse(buf);
        if (root)
        {

            cJSON *jParam = cJSON_GetObjectItem(root, "params");
            if (jParam)
            {
                cJSON *jOOB = cJSON_GetObjectItem(jParam, "OOB");
                if(jOOB)
                {
                    oob = jOOB->valuestring;
                    maix::setFWParaConfig("factory","oob",oob.c_str(),1);
		            maix::saveFWParaConfig();
                }
            }
        }

        return oob;
    }

    int CCliAgentMcuAtbm6441::WifiCmd(std::string strAT)
    {
        std::string  strCmd = "fw_cmd \"" + strAT + "\"";
        return SendToAtbmd(strCmd);
    }

    int CCliAgentMcuAtbm6441::SaveConfig()
    {
        return maix::saveFWParaConfig();
    }

    std::string CCliAgentMcuAtbm6441::GetTxRxInfo()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+GET_TXRX_STAT");
        RecvMcuMsg(3000);

        cli::OutputDevice::GetStdErr() << "GetTxRxInfo:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

    std::string CCliAgentMcuAtbm6441::EnterSleepMode()
    {
        SendToAtbmd("rmmod");

        return "";
    }

    int CCliAgentMcuAtbm6441::FactoryReset()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+SET_FACT_MODE 0");
        RecvMcuMsg(3000);

        cli::OutputDevice::GetStdErr() << "FactoryReset:" << m_strFactoryResponse << cli::endl;

        return 0;
    }

    /* * * * * * * * * API * * * * * * * * * */
	int CCliAgentMcuAtbm6441::RecvSystemPackte(int iId,void *pvData)
    {
        pb_size_t payload_type;
        System *system = (System *)pvData;
    	payload_type = system->which_payload;
		if(iId == System_SystemID_GET_DEVICE_STATUS)
    	{
			DeviceStatus device_status;
			memset(&device_status, 0, sizeof(DeviceStatus));
			memcpy(&device_status, &system->payload.device_status, sizeof(DeviceStatus));

            m_iCapacity = device_status.battery.capacity;

            if(device_status.battery.has_current == 1)
            {
                m_iCurrent = device_status.battery.current;
            }

            if(device_status.battery.has_ntc_temperature == 1)
            {
                m_iTemperature = device_status.battery.ntc_temperature;
            }

            if(device_status.battery.has_voltage = 1)
            {
                m_iVoltage = device_status.battery.voltage;
            }

            cli::OutputDevice::GetStdErr() << "m_iCapacity:"<< m_iCapacity << " m_iCurrent:" << m_iCurrent << " m_iVoltage:" << m_iVoltage << " m_iTemperature:" << m_iTemperature << cli::endl;
    	}
    	else if(iId == System_SystemID_GET_DEVICE_INFO)
    	{
			DeviceInfo device_info;
			memset(&device_info, 0, sizeof(DeviceInfo));
			memcpy(&device_info, &system->payload.device_info, sizeof(DeviceInfo));

            m_strMcuVersion = device_info.firmware_version;
            cli::OutputDevice::GetStdErr() << "Mcu Version:"<< m_strMcuVersion << cli::endl;
    	}
    	else if(iId == System_SystemID_BUTTON_CHANGE)
    	{

    	}
		else
		{
			cli::OutputDevice::GetStdErr() << "RecvSystemPackte is not support id"<< cli::endl;
		}

        return 0;
    }

	int CCliAgentMcuAtbm6441::RecvIpcPackte(int iId,void *pvData)
    {
        pb_size_t payload_type;
        Ipc *ipc = (Ipc *)pvData;
        payload_type = ipc->which_payload;

        if(iId == Ipc_IpcID_REPORT_EVENT)
        {

        }
        else
        {
            cli::OutputDevice::GetStdErr() << "RecvIpcPackte is not support id"<< cli::endl;
        }

        return 0;
    }
    
    int CCliAgentMcuAtbm6441::RecvFactoryPackte(int iId,void *pvData)
    {
        pb_size_t payload_type;
        Factory *factory = (Factory *)pvData;
        payload_type = factory->which_payload;

        if(iId == Factory_FactoryID_TRANSMIT_CMD)
        {
            CmdInfo_Response info;
            memset(&info, 0, sizeof(CmdInfo_Response));
            memcpy(&info, &factory->payload.cmd_info_response, sizeof(CmdInfo_Response));

            char *buf = (char *)malloc(info.data->size + 1);
            if(buf == NULL)
            {
                cli::OutputDevice::GetStdErr() << "RecvFactoryPackte malloc failed"<< cli::endl;
                return -1;
            }
            memcpy(buf, info.data->bytes, info.data->size);
            buf[info.data->size] = '\0';

            m_strFactoryResponse = buf;

            free(buf);
        }
        else
        {
            cli::OutputDevice::GetStdErr() << "RecvFactoryPackte is not support id"<< cli::endl;
        }

        return 0;
    }

    int CCliAgentMcuAtbm6441::RecvHandler(void *pData)
    {
        IotPacket *pkt = (IotPacket *)pData;

        // cli::OutputDevice::GetStdErr() << "id:" << pkt->id <<" type:" << pkt->type << cli::endl;
        switch(pkt->type)
		{
			case IotPacket_Type_SYSTEM:
			{
				System *system = &pkt->payload.system;
				RecvSystemPackte(pkt->id, (void *)system);
			}
			break;
			case IotPacket_Type_IPC:
			{
				Ipc *ipc = &pkt->payload.ipc;
                RecvIpcPackte(pkt->id, (void *)ipc);
			}
			break;
			case IotPacket_Type_FACTORY:
			{
				Factory *factory = &pkt->payload.factory;
                RecvFactoryPackte(pkt->id, (void *)factory);
			}
			break;
			default:
				cli::OutputDevice::GetStdErr() << "it is not support type" << cli::endl;
		}
        return 0;
    }
    
    int CCliAgentMcuAtbm6441::RecvMcuMsg(int iTimeout,std::string debug)
    {
        m_strFactoryResponse = "";
        int ret = RecvBuf(m_pRecvBuf, m_iRecvBufLen, iTimeout);
        if(ret <= 0) 
        {
            cli::OutputDevice::GetStdErr() << debug << " Test RecvBuf failed" << cli::endl;
            return ret;
        }
        
        // 这个函数处理完数据后会调用 commUartRecvHandleCb
        comm_proto_recv_wrapper((char *)m_pRecvBuf, ret);
        return 0;
    }

    int CCliAgentMcuAtbm6441::RecvMcuMsg(int iTimeout)
    {
        m_strFactoryResponse = "";
        int ret = RecvBuf(m_pRecvBuf, m_iRecvBufLen, iTimeout);
        if(ret <= 0) 
        {
            cli::OutputDevice::GetStdErr() << "Test RecvBuf failed" << cli::endl;
            return ret;

        }
        
        // 这个函数处理完数据后会调用 commUartRecvHandleCb
        comm_proto_recv_wrapper((char *)m_pRecvBuf, ret);

        return 0;
    }

    int CCliAgentMcuAtbm6441::GetMcuDeviceState()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        system_device_status_request();
        return RecvMcuMsg(3000,"GetMcuDeviceState");
    }

    int CCliAgentMcuAtbm6441::GetMcuDeviceInfo()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        system_device_info_request();
        return RecvMcuMsg(3000,"GetMcuDeviceInfo");
    }

    std::string CCliAgentMcuAtbm6441::GetMcuVersion()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+GET_APP_VER");
        RecvMcuMsg(3000,"GetMcuVersion");

        cli::OutputDevice::GetStdErr() << "GetMcuVersion:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

    int CCliAgentMcuAtbm6441::GetBattery(int *piCapacity,int *piCurrent,int *piVoltage,int *piTemperature)
    {
        int ret = GetMcuDeviceState();

        if(ret == 0)
        {
            *piCapacity = m_iCapacity;
            *piCurrent = m_iCurrent;
            *piVoltage = m_iVoltage;
            *piTemperature = m_iTemperature;
        }

        return ret;
    }

    int CCliAgentMcuAtbm6441::LedSet(int iIndex,int iFlag)
    {
        Ipc ipc = Ipc_init_default;
        IotPacket packet = IotPacket_init_zero;
        LedInfo led_info;
        size_t data_buf_len = 0;
        pb_istream_t stream;
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);

        led_info.led_color = (LedInfo_Color) iIndex;
        led_info.led_state = (LedInfo_State) iFlag;
        led_info.has_led_brightness = true;
        led_info.led_brightness = 255;

        packet.which_payload = IotPacket_ipc_tag;
        packet.payload.ipc.which_payload = Ipc_led_info_tag;
        memcpy(&packet.payload.ipc.payload.led_info, &led_info, sizeof(LedInfo));
        pb_get_encoded_size(&data_buf_len, IotPacket_fields, &packet);

        // cli::OutputDevice::GetStdErr() << "data_buf_len:" << data_buf_len << " LedInfo:" << sizeof(LedInfo) << cli::endl;

        uint8_t *data_buf = (uint8_t *)malloc(data_buf_len);
        if (data_buf == NULL) {
            cli::OutputDevice::GetStdErr() << "LedSet malloc failed" << cli::endl;
            return -1;
        }
        memset(data_buf, 0, data_buf_len);

        ipc.which_payload = Ipc_led_info_tag;
        memcpy(&ipc.payload.led_info, &led_info, sizeof(LedInfo));

        mx_encode_iot_ipc_proto(Ipc_IpcID_SET_LED_INFO, &ipc, data_buf, data_buf_len);

        mx_proto_send(data_buf, data_buf_len);

        // 现在程序设置led没有返回消息
        // RecvMcuMsg(3000);
        usleep(300*1000);

        free(data_buf);
        return 0;
    }

    std::string CCliAgentMcuAtbm6441::GetBattery()
    {
        static int times = 0;
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+GET_BATT_INFO");
        RecvMcuMsg(3000,"GetBattery");

        if(times++ % 30 == 0)
            cli::OutputDevice::GetStdErr() << "GetBattery:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

    std::string CCliAgentMcuAtbm6441::GetPirSignal()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+GET_PIR_ADC_MAX");
        RecvMcuMsg(3000,"GetPirSignal");

        cli::OutputDevice::GetStdErr() << "GetPirTriggerNum:" << m_strFactoryResponse << cli::endl;

        // SendFactoryCmd("AT+SET_PIR_STATE 1");
        // RecvMcuMsg(3000);

        // cli::OutputDevice::GetStdErr() << "GetPirTriggerNum:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

    std::string CCliAgentMcuAtbm6441::GetPirTriggerNum()
    {
        // SendFactoryCmd("AT+SET_PIR_CNT");
        // RecvMcuMsg(3000);

        // cli::OutputDevice::GetStdErr() << "GetPirTriggerNum:" << m_strFactoryResponse << cli::endl; 
        static int count = 0;
        int tmp_count = 0;
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+GET_PIR_CNT");
        RecvMcuMsg(3000,"GetPirTriggerNum");

        cli::OutputDevice::GetStdErr() << "GetPirTriggerNum:" << m_strFactoryResponse << cli::endl;
        tmp_count = parseNumber(m_strFactoryResponse);
        if(count != tmp_count)
        {
            count=tmp_count;
            return  m_strFactoryResponse;
        }

        return "";
    }

    std::string CCliAgentMcuAtbm6441::StartPirTriggerNum()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+SET_PIR_STATE 2");
        RecvMcuMsg(3000,"StartPirTriggerNum1");

        cli::OutputDevice::GetStdErr() << "GetPirTriggerNum:" << m_strFactoryResponse << cli::endl;

        SendFactoryCmd("AT+SET_PIR_ADC_MAX");
        RecvMcuMsg(3000,"StartPirTriggerNum2");

        cli::OutputDevice::GetStdErr() << "GetPirTriggerNum:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

	std::string  CCliAgentMcuAtbm6441::GetButtonLevel()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        SendFactoryCmd("AT+GET_BUTTON");
        RecvMcuMsg(3000,"GetButtonLevel");

        cli::OutputDevice::GetStdErr() << "GetButtonLevel:" << m_strFactoryResponse << cli::endl;

        return m_strFactoryResponse;
    }

    int CCliAgentMcuAtbm6441::SetBurnInMode(int iMode)
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);
        std::string strCmd = "AT+SET_BURN_IN_TEST ";
        strCmd += std::to_string(iMode);
        SendFactoryCmd(strCmd);
        RecvMcuMsg(3000,"SetBurnInMode");

        cli::OutputDevice::GetStdErr() << "SetBurnInMode:" << m_strFactoryResponse << cli::endl;

        if(m_strFactoryResponse.find("OK") != std::string::npos)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    void ThreadMCUUpgrade(CCliAgentMcuAtbm6441 *mcu)
    {
        int ret = mcu->SendToAtbmd("update_fw /dev/mtd6 1");
        if(ret < 0)
        {
            mcu->m_iUpgradeFlag = -1;
            return;        
        }

        sleep(15);
        mcu->SendToAtbmd("fw_cmd \"AT+LIGHT_SLEEP 1\"");

        system("tag_env_info --set HW 70mai_factory_mode 1");
        sleep(1);
        mcu->m_iUpgradeFlag = 2;
        system("touch /tmp/tag_env_info_lock");
        system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
        sleep(10);
        mcu->SendToAtbmd("fw_cmd \"AT+REBOOT\"");
        mcu->SendFactoryCmd("AT+REBOOT");
    }

    int CCliAgentMcuAtbm6441::MCUUpgrade()
    {
        //升级 
        //SendToAtbmd("AT+WIFI_HEART_PKT TEXT \"{\"'event\"':\"'keapalive\",\"param\":{\"did\":\"123456\",\"time\":\"1000\",\"timeout\":\"1005\"}}\" PERIOD 6000 SERVER 192.168.80.242 PORT 55111");
        if(m_iUpgradeFlag == 0)
        {
            m_iUpgradeFlag = 1;
            static std::thread threadMCUUpgrade(ThreadMCUUpgrade,this);
            return 0;
        }
        else if(m_iUpgradeFlag == 1)
        {
            return 1;
        }

        else if(m_iUpgradeFlag == 2)
        {
            return 2;
        }

        return -1;
    }

    int CCliAgentMcuAtbm6441::SysSHIPMode()
    {
        maix::saveFWParaConfig();
        system("fw_setenv lock");
        SendToAtbmd("fw_cmd \"AT+SYS_SHIP_MODE\"");
        SendFactoryCmd("AT+SYS_SHIP_MODE");
        RecvMcuMsg(3000,"SYS_SHIP_MODE");
        return 0;
    }

    int CCliAgentMcuAtbm6441::SolarTestStart()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);

        SendFactoryCmd("AT+SET_SOLAR_TEST");
        RecvMcuMsg(3000);

        cli::OutputDevice::GetStdErr() << "SolarTestStart:" << m_strFactoryResponse << cli::endl;

        return 0;
    }

    int CCliAgentMcuAtbm6441::SolarTestGetResult()
    {
        std::lock_guard<std::mutex> lock(m_mutexMcuUart);

        SendFactoryCmd("AT+GET_SOLAR_TEST");
        RecvMcuMsg(3000);
        int curAvg = 0;
        sscanf(m_strFactoryResponse.c_str(),"+FACT:cur_avg:%d" ,&curAvg);
        cli::OutputDevice::GetStdErr() << "SolarTestStart:" << m_strFactoryResponse << cli::endl;

        return curAvg;
    }

    void CCliAgentMcuAtbm6441::ThreadLed()
    {
        while(!m_bshouldExit)
        {
            LedSet(0,1);
            usleep(300000);
            LedSet(2,1);
            usleep(300000);
            LedSet(1,1);
            usleep(300000);
        }
    }
    void CCliAgentMcuAtbm6441::StartLed()
    {
        m_bshouldExit = false;
        if (!m_bRunning) 
        {
            m_ledthread = std::thread([this] 
            {
                this->ThreadLed();
            });
            m_bRunning = true;
        }
    }

    void CCliAgentMcuAtbm6441::StopLed()
    {
        m_bshouldExit = true;
        if (m_bRunning) 
        {
            if (m_ledthread.joinable()) 
                m_ledthread.join();
            m_bRunning = false;
        }
    }
}
