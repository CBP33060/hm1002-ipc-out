#include "mcu_serial_port_module.h"
#include "com_rpc_client_end_point.h"
#include "cJSON.h"
#include "com_rpc_server_end_point.h"
#include "mcu_serial_port_remote_event_server.h"
#include "log_mx.h"
#include "mx_proto_decode.h"
#include "mx_comm_adapter.h"
#include "mx_proto_system.h"
#include "power_up_sound.h"
#include "mx_proto_ipc.h"
#include "mbedtls/base64.h"
#include "pb_encode.h"
#include "mx_proto_encode.h"
#include "mx_proto_common.h"
#include "mx_proto_factory.h"
#include "fw_env_para.h"
#include "common.h"
#include <sys/prctl.h>
#include <pthread.h>
using namespace maix;

maix::CMCUSerialPortModule * g_moudle;

int32_t commUartSendCb(void *data, uint16_t len);
int32_t commUartRecvCb(IotPacket *pkt);

int32_t commUartSendCb(void *data, uint16_t len)
{
	if(g_moudle != NULL)
		g_moudle->uartSendCb(data, len);
	return 0;
}

int32_t commUartRecvCb(IotPacket *pkt)
{
	if(g_moudle != NULL)
		g_moudle->uartRecvCb(pkt);
	return 0;
}

namespace maix {
	CMCUSerialPortModule::CMCUSerialPortModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
		, m_pcRecvBuffer(NULL)
		, m_iRecvBufferLen(1024)
		, m_acRecvIndex(0)
	{
		g_moudle = this;

		m_iCapacity = 0;
		m_iCurrent = 0;
		m_iVoltage = 0;
		m_iTemperature = 0;
		m_ichargeStatus = 0;
		m_iflag = 0;
		m_iaesFlag = 0;
		m_iledState = 1;
        m_iForceLowpowerCount = 0;

		m_keyPress = mxfalse;
		m_netWakeUp = mxfalse;
		m_KeyCount = 0;
		m_NetCount = 0;

		memset(&m_keepAliveConfig, 0, sizeof(m_keepAliveConfig));
	}

	CMCUSerialPortModule::~CMCUSerialPortModule()
	{
		if (m_pcRecvBuffer != NULL)
		{
			free(m_pcRecvBuffer);
			m_pcRecvBuffer = NULL;
		}
	}

	mxbool CMCUSerialPortModule::init()
	{
		if (!getConfig("PORT_CONFIG", "NAME", m_strPortName))
		{
			return mxfalse;
		}

		if (!initConnectModule())
			return mxfalse;

		if (!initMcuManageRemoteEvent())
			return mxfalse;

		if (!m_objSendQueue.init(10, 0))
			return mxfalse;

		if (!initServer())
			return mxfalse;

		m_cSerialPort.connectReadEvent(this);

		m_cSerialPort.setReadIntervalTimeout(1);
		m_cSerialPort.init(m_strPortName.c_str(),
			BaudRate115200);
		m_cSerialPort.open(1);

		if (m_cSerialPort.isOpen())
		{
			logPrint(MX_LOG_INFOR, "port %s open success",
				m_strPortName.c_str());
		}
		else
		{
			logPrint(MX_LOG_INFOR, "port %s is in use",
				m_strPortName.c_str());
		}

		if (!m_pcRecvBuffer)
		{
			m_pcRecvBuffer = (char*)malloc(m_iRecvBufferLen);
			if (!m_pcRecvBuffer)
				return mxfalse;
		}

		m_threadSendProc = std::thread([this]() {
			this->run();
		});

		comm_proto_init_wrapper(&commUartSendCb);
		mx_proto_packet_rcv_cb_register(&commUartRecvCb);
		system_device_status_request();
		usleep(20*1000);
        // sendMessageToEvent(TYPE_EVENT_MANAGE_PIR_EVENT,"");
		//ipc_dev_config_request();
		if (!initDevSpecAdapter())
		{
			return mxfalse;
		}

        // std::thread([this]() {
        //     while (true)
        //     {
        //         usleep(2 * 1000 * 1000);
        //         playAudioFile("warming_alarm",2,0);
        //         usleep(10 * 1000 * 1000);
        //         playAudioFile("low_power",4,0);
        //         usleep(30 * 1000 * 1000);
        //         playAudioFile("reset_success",3,0);
        //         usleep(30 * 1000 * 1000);
        //         playAudioFile("wifi_connecting",2,0);
        //         usleep(30 * 1000 * 1000);
        //         playAudioFile("sdcard_err",1,0);
        //         usleep(30 * 1000 * 1000);
        //     }

        // }).detach();
        m_threadForceEnterLowPower = std::thread([this]() {
			this->forceEnterLowpower();
		});

		return mxtrue;
	}

	mxbool CMCUSerialPortModule::unInit()
	{
		if (m_pcRecvBuffer != NULL)
		{
			free(m_pcRecvBuffer);
			m_pcRecvBuffer = NULL;
		}

		return mxtrue;
	}

	mxbool CMCUSerialPortModule::initServer()
	{
		int iComServerConfigNum;
		if (!getConfig("COM_SERVER_CONFIG", "NUM", iComServerConfigNum))
		{
			return mxfalse;
		}

		for (int j = 0; j < iComServerConfigNum; j++)
		{
			char acComServerConfig[64] = { 0 };
			snprintf(acComServerConfig, sizeof(acComServerConfig),
				"COM_SERVER_CONFIG_%d", j);

			std::string strName;
			if (!getConfig(acComServerConfig, "NAME", strName))
			{
				return mxfalse;
			}

			if (!isClientExist(strName))
			{
				std::string strComType;
				if (!getConfig(acComServerConfig, "COM_TYPE", strComType))
				{
					return mxfalse;
				}

				if (strComType.compare("RCF_EVENT") == 0)
				{
					int iType;
					if (!getConfig(acComServerConfig, "TYPE", iType))
					{
						return mxfalse;
					}

					std::string strIP;
					if (!getConfig(acComServerConfig, "IP", strIP))
					{
						return mxfalse;
					}

					int iPort;
					if (!getConfig(acComServerConfig, "PORT", iPort))
					{
						return mxfalse;
					}

					std::string strUnix;
					if (!getConfig(acComServerConfig, "UNIX", strUnix))
					{
						return mxfalse;
					}

					int iLen;
					if (!getConfig(acComServerConfig, "LEN", iLen))
					{
						return mxfalse;
					}

					if (strName.compare("remote_event_server") == 0)
					{
						CMCUSerialPortRemoteEventServer * objMCUSerialPortRemoteEventServer =
							new CMCUSerialPortRemoteEventServer(this);

						if (!objMCUSerialPortRemoteEventServer)
							return mxfalse;

						objMCUSerialPortRemoteEventServer->init();

						T_COM_PROXY_SERVER_CONFIG serverConfig;
						serverConfig.m_iType = iType;
						serverConfig.m_strIP = strIP;
						serverConfig.m_iPort = iPort;
						serverConfig.m_strUnix = strUnix;
						serverConfig.m_iUDPMsgLen = iLen;
						serverConfig.m_iTCPMsgLen = iLen;
						serverConfig.m_iUNIXMsgLen = iLen;
                        serverConfig.m_iTCPConLimit = 10;
                        serverConfig.m_iUDPConLimit = 10;
                        serverConfig.m_iUNIXConLimit = 10;

						std::shared_ptr<CComRcfServerEndPoint> objServerEndPoint(
							new CComRcfServerEndPoint(this));

						objServerEndPoint->init(serverConfig,
							objMCUSerialPortRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}

		}

		return mxtrue;
	}

	mxbool CMCUSerialPortModule::initConnectModule()
	{
		int iConnectModuleNum = 0;
		if (!getConfig("CONNECT_MODULE", "NUM", iConnectModuleNum))
		{
			return mxfalse;
		}

		for (int i = 0; i < iConnectModuleNum; i++)
		{
			char acConnectModule[64] = { 0 };
			snprintf(acConnectModule, sizeof(acConnectModule),
				"CONNECT_MODULE_%d", i);

			std::string  strName;
			if (!getConfig(acConnectModule, "NAME", strName))
			{
				return mxfalse;
			}

			std::string  strGUID;
			if (!getConfig(acConnectModule, "GUID", strGUID))
			{
				return mxfalse;
			}

			std::string  strConfig;
			if (!getConfig(acConnectModule, "CONFIG", strConfig))
			{
				return mxfalse;
			}

			std::shared_ptr<CIModule> imodule = NULL;
			if (!getConnectModule(strGUID, imodule))
			{
				std::shared_ptr<CModule> newModule(
					new CModule(strGUID, strName));

				if (!newModule->loadConfig(strConfig))
					return mxfalse;

				imodule = newModule;
				if (!connect(imodule))
					return mxfalse;
			}

			CModule *module = dynamic_cast<CModule *>(imodule.get());
			int iComServerConfigNum;
			if (!module->getConfig("COM_SERVER_CONFIG", "NUM", iComServerConfigNum))
			{
				return mxfalse;
			}

			for (int j = 0; j < iComServerConfigNum; j++)
			{
				char acComServerConfig[64] = { 0 };
				snprintf(acComServerConfig, sizeof(acComServerConfig),
					"COM_SERVER_CONFIG_%d", j);

				std::string strName;
				if (!module->getConfig(acComServerConfig, "NAME", strName))
				{
					return mxfalse;
				}

				if (!module->isClientExist(strName))
				{
					std::string strComType;
					if (!module->getConfig(acComServerConfig, "COM_TYPE", strComType))
					{
						return mxfalse;
					}

					if (strComType.compare("RCF_EVENT") == 0)
					{
						int iType;
						if (!module->getConfig(acComServerConfig, "TYPE", iType))
						{
							return mxfalse;
						}

						std::string strIP;
						if (!module->getConfig(acComServerConfig, "IP", strIP))
						{
							return mxfalse;
						}
						else
						{
							if (strIP.compare("0.0.0.0") == 0)
							{
								strIP = std::string("127.0.0.1");
							}
						}

						int iPort;
						if (!module->getConfig(acComServerConfig, "PORT", iPort))
						{
							return mxfalse;
						}

						std::string strUnix;
						if (!module->getConfig(acComServerConfig, "UNIX", strUnix))
						{
							return mxfalse;
						}

						int iLen;
						if (!module->getConfig(acComServerConfig, "LEN", iLen))
						{
							return mxfalse;
						}

						std::shared_ptr<CComRCFClientEndPoint> objClientEndPoint(
							new CComRCFClientEndPoint(this));

						T_COM_PROXY_CLIENT_CONFIG clientConfig;


						clientConfig.m_iType = iType;
						clientConfig.m_strIP = strIP;
						clientConfig.m_iPort = iPort;
						clientConfig.m_strUnix = strUnix;
						clientConfig.m_iTCPMsgLen = iLen;
						clientConfig.m_iUDPMsgLen = iLen;
						clientConfig.m_iUNIXMsgLen = iLen;

						objClientEndPoint->init(clientConfig, E_CLIENT_EVENT);
						if (!module->regClient(strName, objClientEndPoint))
							return mxfalse;
					}
				}
			}

		}

		return mxtrue;
	}

	std::string CMCUSerialPortModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		if (0 == strEvent.compare("EnterLowPower"))
		{
			return setMCUEnterLowPower(strParam);
		}
		else if (0 == strEvent.compare("ipcManageEvent"))
		{
			return sendAesMessageToMCU(strParam);
		}
		else if (0 == strEvent.compare("SetLedSwitch"))
        {
            return setMcuLedSwitch(strParam);
        }
		else if (0 == strEvent.compare("SetLedStatus"))
        {
            return setMcuLedStatus(strParam);
        }
		else if (0 == strEvent.compare("StrPirStatus") || 
				(0 == strEvent.compare("StrPirlnterva")) || 
				(0 == strEvent.compare("StrPirSensitivity")))
        {
            return setMcuPirStatus(strParam);
        }
		else if (0 == strEvent.compare("GetBatteryLevel"))
        {
            return getBatteryCapacity();
        }
		else if (0 == strEvent.compare("GetChargingState"))
        {
            return getBatteryChargingState();
        }
		else if (0 == strEvent.compare("SendSpeakerBroadcast"))
        {
            return sendSpeakerVoice(strParam);
        }
        else if (0 == strEvent.compare("LogManageOnline"))
        {
            return setLogManageOnlineState(strParam);
        }
        else if (0 == strEvent.compare("GetPowerUpPlayEndState"))
        {
            return getPowerUpPlayEndState();
        }
		else if (0 == strEvent.compare("ResetIPC"))
		{
			m_timeReset.StartTimerOnce(2 * 1000,
				std::bind(&CMCUSerialPortModule::makeToReset, this));
			return procResult(std::string("200"), "", "reset ipc ok");
		}
		else if(0 == strEvent.compare("GetDevInfo"))
		{
			return m_devSpecAdapter->getAllDevInfo();
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	mxbool CMCUSerialPortModule::sendSpecDataVoiceLight()
	{
		cJSON* jsonRoot = cJSON_CreateObject();
		cJSON* jsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonParam, "configName", "VoiceLightWarning");
		cJSON_AddStringToObject(jsonParam, "configValue", "6");
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
		cJSON_AddStringToObject(jsonRoot, "event", "sendSpecData");
		std::string strData = cJSON_Print(jsonRoot);

		std::string strResult = output(m_strEventManageGUID, m_strEventManageRemoteEventServer, (unsigned char*)strData.c_str(), strData.length());

		if(jsonRoot)
		{
			cJSON_free(jsonParam);
			jsonRoot = NULL;
		}
		return mxtrue;
	}

    std::string CMCUSerialPortModule::getPowerUpPlayEndState()
    {
        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonParam = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "GetPowerUpPlayEndState");

        cJSON_AddBoolToObject(jsonParam, "endState", CPowerUpSound::getInstance().getPlayEndState());

        cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        char *out = cJSON_Print(jsonRoot);
        std::string strState = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        return procResult("200", strState, "");
    }

	std::string CMCUSerialPortModule::procResult(
		std::string code, std::string strMsg, std::string strErr)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonMsg = cJSON_Parse(strMsg.c_str());
		cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
		cJSON_AddItemToObject(jsonRoot, "msg", jsonMsg);
		cJSON_AddStringToObject(jsonRoot, "errMsg", strErr.c_str());
		char *pcResult = cJSON_Print(jsonRoot);
		strResult = std::string(pcResult);
		cJSON_Delete(jsonRoot);
		if (pcResult)
			free(pcResult);

		return strResult;
	}

	void CMCUSerialPortModule::onReadNoBufEvent(const char *portName, char * readBuffer, unsigned int readBufferLen)
	{
		// printf("Aaa:%d\n",readBufferLen);
		if (m_strPortName.compare(portName) == 0)
		{
			if (readBuffer && readBufferLen > 0)
			{
				comm_proto_recv_wrapper((char *)readBuffer, readBufferLen);
			}		
		}
	}

	void CMCUSerialPortModule::onReadEvent(const char * portName, 
		unsigned int readBufferLen)
	{
		// printf("bbb\n");
		if (m_strPortName.compare(portName) == 0)
		{
			if (readBufferLen > (unsigned int)m_iRecvBufferLen)
			{
				if (m_pcRecvBuffer)
				{
					free(m_pcRecvBuffer);
					m_pcRecvBuffer = NULL;
				}
					
				m_pcRecvBuffer = (char*)malloc(readBufferLen);
				if (!m_pcRecvBuffer)
				{
					printf("WCQ=malloc failed\n");
					return;
				}
				m_iRecvBufferLen = readBufferLen;
			}

			if (m_pcRecvBuffer)
			{
				memset(m_pcRecvBuffer, 0, m_iRecvBufferLen);
				int recLen = m_cSerialPort.readData(m_pcRecvBuffer, readBufferLen);

				// printf("rx=%d\r\n",recLen);
				// for (int i = 0; i < recLen; i++)
				// {
				// 	printf("%02x ", (unsigned char)m_pcRecvBuffer[i]);
				// 	if(i != 0 && i % 16 == 0){
				// 		printf("\n");
				// 	}
				// }
				// printf("\n");

				comm_proto_recv_wrapper((char *)m_pcRecvBuffer, recLen);
			}		
		}
	}

	mxbool CMCUSerialPortModule::pushSendData(
		std::shared_ptr<T_SerialPortData> &data)
	{
		return  m_objSendQueue.push(data);
	}

	void CMCUSerialPortModule::popSendData(
		std::shared_ptr<T_SerialPortData> &data)
	{
		m_objSendQueue.pop(data);
	}

	mxbool CMCUSerialPortModule::sendData(std::shared_ptr<T_SerialPortData> data)
	{
		return pushSendData(data);
	}
	
	mxbool CMCUSerialPortModule::commSendData(char *sdata, unsigned int dataLen)
	{
		std::shared_ptr<T_SerialPortData> data(new T_SerialPortData);
		memset(data.get(), 0, sizeof(T_SerialPortData));
		data->m_iEventID = 1;
		memcpy((char *)data->m_ucData, (char *)sdata, dataLen);
		data->m_iLen = dataLen;
		// printf("sd=%d\n", dataLen);
		// for (int i = 0; i < dataLen; i++)
		// {
		// 	printf("%02x ", (unsigned char)sdata[i]);
		// }
		// printf("\n");
		return sendData(data);
	}

	int32_t CMCUSerialPortModule::uartSendCb(void *data, uint16_t len)
	{
		// uint8_t *dataBuf = (uint8_t *)data;

		commSendData((char *)data, len);
		return 0;
	}

	mxbool CMCUSerialPortModule::commRecvSystemData(System_SystemID id, System *system)
	{
		// pb_size_t payload_type;
    	// payload_type = system->which_payload;
		if(id == System_SystemID_GET_DEVICE_STATUS)
    	{
			DeviceStatus device_status;
			memset(&device_status, 0, sizeof(DeviceStatus));
			memcpy(&device_status, &system->payload.device_status, sizeof(DeviceStatus));
			//设备唤醒原因
			logPrint(MX_LOG_INFOR, "device_status.wakeup_host_reason::[%d]\r\n",device_status.wakeup_host_reason);
			printf("device_status.wakeup_host_reason::[%d]\r\n",device_status.wakeup_host_reason);
			if(m_iflag == 0)
			{
				if(device_status.wakeup_host_reason == DeviceStatus_WakeupHostReason_WAKEUP_TYPE_IO)
				{
					std::string strResetValue;
					int iPos = -1;
					int getResetValue= 0;

					linuxPopenExecCmd(strResetValue, "tag_env_info --get HW reset_value");
					iPos = strResetValue.find("Value=");
					if (iPos > 0) 
					{
						getResetValue= std::stoi(strResetValue.substr(iPos + strlen("Value="), strResetValue.length() - strlen("Value=")));
					}else
					{
						getResetValue = 0;
					}
					if(getResetValue)
					{
						logPrint(MX_LOG_INFOR, "The device is in the process of restarting after a reset without announcing\r\n");
						linuxPopenExecCmd(strResetValue, "tag_env_info --set HW reset_value 0");
					}else
					{
						//开机音效，接收状态为type_io
						CPowerUpSound::getInstance().startPlaySound();
						{
							std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
							m_keyPress = mxtrue;
							m_KeyCount= 0;
						}
						
						if (!m_threadDelayLowPower.joinable()) {
							m_threadDelayLowPower = std::thread([this]() {
								this->DelayLowPowerRun();
							});
						}
					}
				}
				if(device_status.wakeup_host_reason == DeviceStatus_WakeupHostReason_WAKEUP_TYPE_PIR)
				{
					//上报pir事件
					sendMessageToEvent(TYPE_EVENT_MANAGE_PIR_EVENT,std::to_string(device_status.wakeup_host_reason));
				} 
				if(device_status.wakeup_host_reason == DeviceStatus_WakeupHostReason_WAKEUP_TYPE_TCP_NETPATTERN)
				{
					std::string strValue;
					linuxPopenExecCmd(strValue, "touch /tmp/_netWakeUp");
					//上报网络唤醒事件
					{
						std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
						m_netWakeUp = mxtrue;
						m_NetCount= 0;
					}
					
					if (!m_threadDelayLowPower.joinable()) {
						m_threadDelayLowPower = std::thread([this]() {
							this->DelayLowPowerRun();
						});
					}
					sendMessageToEvent(TYPE_EVENT_MANAGE_NET_EVENT, std::to_string(device_status.wakeup_host_reason));
				} 

				if(device_status.wakeup_host_reason == DeviceStatus_WakeupHostReason_WAKEUP_TYPE_KEY)
				{
					//低功耗下按键唤醒
					{
						std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
						m_keyPress = mxtrue;
						m_KeyCount= 0;
					}
					
					if (!m_threadDelayLowPower.joinable()) {
						m_threadDelayLowPower = std::thread([this]() {
							this->DelayLowPowerRun();
						});
					}
			
				} 

				m_iflag = 1;
			}

			m_iCapacity = device_status.battery.capacity;
			if(device_status.battery.has_current == 1)
            {
                m_iCurrent = device_status.battery.current;
            }

			if(device_status.battery.has_voltage == 1)
            {
                m_iVoltage = device_status.battery.voltage;
            }

			if(device_status.battery.has_ntc_temperature == 1)
            {
                m_iTemperature = device_status.battery.ntc_temperature;
            }
			//电池状态标志未修复
			// if(device_status.battery.has_charge_status == 1)
			// {
				m_ichargeStatus = device_status.battery.charge_status;
			// }
			//依次为：电池容量  电流  电压  温度 充电状态
			logPrint(MX_LOG_INFOR, "Mcu battery info:[%d],[%d],[%d],[%d],[%d]\r\n",m_iCapacity, m_iCurrent, m_iVoltage, m_iTemperature, m_ichargeStatus);

			std::unique_lock<std::mutex> lock(m_mutexSendBatteryInfo);
			m_conditionBatteryInfo.notify_one();
    	}
    	else if(id == System_SystemID_GET_DEVICE_INFO)
    	{
			DeviceInfo device_info;
			memset(&device_info, 0, sizeof(DeviceInfo));
			memcpy(&device_info, &system->payload.device_info, sizeof(DeviceInfo));
			//还无法获取版本号
			m_strMcuVersion = device_info.firmware_version;
			logPrint(MX_LOG_INFOR, "Mcu Version is:[%d]\r\n",m_strMcuVersion);
    	}
    	else if(id == System_SystemID_BUTTON_CHANGE)
    	{
			Button button;
			memset(&button, 0, sizeof(Button));
			memcpy(&button, &system->payload.button, sizeof(Button));
			/* add do button event */
			printf("button.button_status is ::[%d]\r\n",button.button_status );
			if(button.button_status)
			{
				// do somethings;
			}
    	}
		else
		{
			logPrint(MX_LOG_INFOR, "it is not support id:[%d]",id);
		}
		return mxtrue;
	}

	void CMCUSerialPortModule::commRecvIPCData(Ipc_IpcID id, Ipc *ipc)
    {
        // pb_size_t payload_type;
        // payload_type = ipc->which_payload;
		logPrint(MX_LOG_INFOR, "payload type id is::[%d]",id);
        if(id == Ipc_IpcID_REPORT_EVENT)
        {
            ReportEvent reminder_event;
            memset(&reminder_event, 0, sizeof(ReportEvent));
            memcpy(&reminder_event, &ipc->payload.report_event, sizeof(ReportEvent));

			logPrint(MX_LOG_INFOR, "reminder_event.type::[%d]\r\n",reminder_event.type);
			printf("reminder_event.type::[%d]\r\n",reminder_event.type);

            if(reminder_event.type == ReportEvent_Type_BUTTON_SHUTDOWN || reminder_event.type == ReportEvent_Type_LOW_POWER_SHUTDOWN || reminder_event.type == ReportEvent_Type_NTC_SHUTDOWN || reminder_event.type == ReportEvent_Type_SDIO_ALIVE_REBOOT || reminder_event.type == ReportEvent_Type_NORMAL_REBOOT)
            {
				if(reminder_event.type == ReportEvent_Type_BUTTON_SHUTDOWN)
				{
					//播放关机音频
                	playAudioFile(PLAY_FILE_SHUT_DOWN,4,0);
				}
				system("touch /tmp/tag_env_info_lock");
				system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
				{
					std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
					m_keyPress = mxtrue;
					m_KeyCount= 0;
				}
				
				if (!m_threadDelayLowPower.joinable()) {
					m_threadDelayLowPower = std::thread([this]() {
						this->DelayLowPowerRun();
					});
				}
            } 
			else if(reminder_event.type == ReportEvent_Type_BUTTON_FACTORY_RESET)
			{
				//播放重置音频并进行重置操作
				if(!makeToReset())
				{
					logPrint(MX_LOG_ERROR, "Script execution failed\n");
				}
			}
			else if((reminder_event.type == ReportEvent_Type_BUTTON_PRESS) || (reminder_event.type == ReportEvent_Type_NET_WAKEUP))
			{
				{
					std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
					m_keyPress = mxtrue;
					m_KeyCount = 0;
				}

				if (!m_threadDelayLowPower.joinable()) {
					m_threadDelayLowPower = std::thread([this]() {
						this->DelayLowPowerRun();
					});
				}
			}
			else if(reminder_event.type == ReportEvent_Type_BUTTON_RELEASE)
			{
				std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
				// m_keyPress = mxfalse;
				m_KeyCount = 0;
			}
			else if (reminder_event.type == ReportEvent_Type_ENTER_FACTORY_MODE)
			{
				//重启T41和mcu
				FactoryReset();
			}
        }
		else if (id == Ipc_IpcID_GET_DEV_CONFIG)
		{
			m_devSpecAdapter->handleDevConfig(ipc);
		}
		else if((id == Ipc_IpcID_KEEPALIVE_CONFIG) && (m_iaesFlag == 0))
		{
				m_iaesFlag = 1;
				logPrint(MX_LOG_INFOR, "ipc set aes key ack successfully");
		}
        else
        {
            logPrint(MX_LOG_INFOR, "it is not support id:[%d]",id);
        }
    }

	int32_t CMCUSerialPortModule::uartRecvCb(IotPacket *pkt)
	{
		printf("support id is:[%d]support type is:[%d]\r\n", pkt->id, pkt->type);
		switch(pkt->type)
		{
			case IotPacket_Type_SYSTEM:
			{
				System *system = &pkt->payload.system;
				commRecvSystemData((System_SystemID)pkt->id, system);

			}
			break;
			case IotPacket_Type_IPC:
			{
				Ipc *ipc = &pkt->payload.ipc;
                commRecvIPCData((Ipc_IpcID)pkt->id, ipc);
			}
			break;
			case IotPacket_Type_FACTORY:
			{
				// Factory *factory = &pkt->payload.factory;
			}
			break;
			default:
				logPrint(MX_LOG_INFOR, "it is not support type:[%d]", pkt->type);
		}
		return 0;
	}

	void CMCUSerialPortModule::run()
	{
		// int i = 0;
		if (prctl(PR_SET_NAME, "CMCUSerialPortModule") != 0) {
			perror("prctl");
			return;
		}
		// printf("WCQ==%s==PID===%d\n",__func__,getpid());
		while (1)
		{
			// if(i++ % 2 == 0) {
			// 	system_device_status_request();
			// } else {
			// 	ipc_dev_config_request();
			// }
			// if(i % 10 == 0){
			// 	// sleep(1);
			// }
			std::shared_ptr<T_SerialPortData> data = NULL;
			popSendData(data);
			if (!data)
			{
#ifdef	WIN32
				Sleep(500);
#else
				usleep(1000 * 500);
#endif	
				continue;
			}

			m_cSerialPort.writeData(data->m_ucData, data->m_iLen);
		}
	}

	mxbool CMCUSerialPortModule::initMcuManageRemoteEvent()
	{
        m_mcuManageRemoteEvent = std::shared_ptr<CMcuManageRemoteEvent>(new CMcuManageRemoteEvent(this));

		if (!m_mcuManageRemoteEvent->init())
		{
			return mxfalse;
		}

        if (!getConfig("EVENT_MANAGE_REMOTE_EVENT", "GUID", m_strEventManageGUID))
		{
			return mxfalse;
		}
		
		if (!getConfig("EVENT_MANAGE_REMOTE_EVENT", "SERVER", 
			m_strEventManageRemoteEventServer))
		{
			return mxfalse;
		}

        if (!getConfig("DEV_MANAGE_REMOTE_EVENT", "GUID", m_strDevManageGUID))
        {
            return mxfalse;
        }
        
        if (!getConfig("DEV_MANAGE_REMOTE_EVENT", "SERVER", 
            m_strDevManageServer))
        {
            return mxfalse;
        }

		return mxtrue;
	}

	mxbool CMCUSerialPortModule::initDevSpecAdapter()
	{
		m_devSpecAdapter = std::shared_ptr<CMcuDecSpecAdapter>(new CMcuDecSpecAdapter(this));
		if (!m_devSpecAdapter->init())
		{
			logPrint(MX_LOG_ERROR, "initDevSpecAdapter failed");
			return mxfalse;
		}

		logPrint(MX_LOG_DEBUG, "initDevSpecAdapter success");

		return mxtrue;
	}

    mxbool CMCUSerialPortModule::sendMessageToEvent(std::string strEventType,std::string value)
    {
        return sendMessageToOtherModule(m_strEventManageGUID,m_strEventManageRemoteEventServer,strEventType,value);
    }

    mxbool CMCUSerialPortModule::sendMessageToDev(std::string strEventType,std::string value)
    {
        return sendMessageToOtherModule(m_strDevManageGUID,m_strDevManageServer,strEventType,value);
    }
    mxbool CMCUSerialPortModule::sendMessageToOtherModule(std::string strGUID,std::string strServer,std::string strEventType,std::string value)
    {
        std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT>  outPut(
                    new T_MCU_MANAGE_RENOTE_OUTPUT());
        outPut->strGUid = strGUID;
        outPut->strServer = strServer;
        outPut->strEvent = OUT_PUT_EVENT_MANAGE_EVENT;

        cJSON *jsonParam = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonParam, "eventType", strEventType.c_str());
        cJSON_AddStringToObject(jsonParam, "value", (value.empty() ? "" : value.c_str()));
        char *out = cJSON_Print(jsonParam);
        std::string strJsonParam = std::string(out);
        cJSON_Delete(jsonParam);
        if (out)
            free(out);
        outPut->strJsonParam = strJsonParam;

        return m_mcuManageRemoteEvent->pushRecvData(outPut);
    }

    mxbool CMCUSerialPortModule::playAudioFile(std::string fileId,int level,int playTime)
    {
        std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT>  outPut(
					new T_MCU_MANAGE_RENOTE_OUTPUT());
        outPut->strGUid = m_strDevManageGUID;
        outPut->strServer = m_strDevManageServer;
        outPut->strEvent = OUT_PUT_DEV_MANAGE_PLAY_FILE_EVENT;

        cJSON *jsonParam = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonParam, "fileId", fileId.c_str());
        cJSON_AddNumberToObject(jsonParam,"level",level);
        cJSON_AddNumberToObject(jsonParam,"playTime",playTime);
        char *out = cJSON_Print(jsonParam);
        std::string strJsonParam = std::string(out);
        cJSON_Delete(jsonParam);
        if (out)
            free(out);
        outPut->strJsonParam = strJsonParam;
        return m_mcuManageRemoteEvent->pushRecvData(outPut);
    }

    mxbool CMCUSerialPortModule::stopAudioFile(std::string fileId)
    {
        std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT>  outPut(
                new T_MCU_MANAGE_RENOTE_OUTPUT());
        outPut->strGUid = m_strDevManageGUID;
        outPut->strServer = m_strDevManageServer;
        outPut->strEvent = OUT_PUT_DEV_MANAGE_STOP_PLAY_FILE_EVENT;

        cJSON *jsonParam = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonParam, "fileId", fileId.c_str());
        char *out = cJSON_Print(jsonParam);
        std::string strJsonParam = std::string(out);
        cJSON_Delete(jsonParam);
        if (out)
            free(out);
        outPut->strJsonParam = strJsonParam;
        return m_mcuManageRemoteEvent->pushRecvData(outPut);
    }

	std::string CMCUSerialPortModule::setMCUEnterLowPower(std::string strParam)
	{
		//给MCU发送关机事件,给T41断电(睡眠模式)
		// SystemMode mode;
		// mode.system_mode = 0;
		// system_set_system_mode(&mode);
		std::string resultMsg;

		if(access("/tmp/_liveing",F_OK) == 0)
		{
			resultMsg = procResult(std::string("500"), "", "liveing");
			return resultMsg;
		}

		if(m_keyPress || m_netWakeUp)
		{
			resultMsg = procResult(std::string("500"), "", "device is keyPress runing");
			return resultMsg;
		}

		char *lowpower_mode = getFWParaConfig("user", "lowpower_mode");
		if(lowpower_mode != NULL)
		{
			printf("lowpower_mode :%d\n", atoi(lowpower_mode));
            logPrint(MX_LOG_ERROR, "lowpower_mode : %d\n", atoi(lowpower_mode));
			if(atoi(lowpower_mode) == 0)
			{
				resultMsg = procResult(std::string("500"), "", "device is not lowpower mode");
				return resultMsg;
			}

			if(!access("/tmp/enter_shipmode", F_OK))
			{
				// printf("WCQ===aaa\n");
				SystemMode mode;
				mode.system_mode = 1;
				system("touch /tmp/tag_env_info_lock");
				system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
				sleep(3);
				system_set_system_mode(&mode);
				resultMsg = procResult(std::string("200"), "", "device is shutdown");
				return resultMsg;
			}
		}
		else
		{
			logPrint(MX_LOG_ERROR, "getFWParaConfig get lowpower_mode failed\n");
			printf("lowpower_mode is null\n");
		}
		system("touch /tmp/tag_env_info_lock");
        system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
		sleep(1);
		if (chdir("/usr/wifi/") != 0) 
		{
			resultMsg = procResult(std::string("500"), "", "Failed to change directory");
			return resultMsg;
		}

		int ret = system("./atbm_iot_cli rmmod");
		if (ret != 0) 
		{
			resultMsg = procResult(std::string("500"), "", "Failed to execute command");
			return resultMsg;
		}
		
		resultMsg = procResult(std::string("200"), "", "EnterLowPower success");
		return resultMsg;
	}

    void CMCUSerialPortModule::forceEnterLowpower()
    {
        while (1)
        {
#ifdef	WIN32
            Sleep(100);
#else
            usleep(10 * 1000 * 1000);
#endif
            m_iForceLowpowerCount ++;
            if(access("/tmp/_liveing",F_OK) == 0)
            {
                system("rm /tmp/force_enter_lowpower");
            }
            else if(m_iForceLowpowerCount > 15)
            {
                logPrint(MX_LOG_INFOR,"CMCUSerialPortModule touch enter lowpower");
                system("touch /tmp/force_enter_lowpower");
            }
            if(access("/tmp/force_enter_lowpower",F_OK) != 0)
            {
                continue;
            }
			if(m_keyPress || m_netWakeUp)
			{
				continue;
			}
            char *lowpower_mode = getFWParaConfig("user", "lowpower_mode");
            if(lowpower_mode != NULL)
            {
                printf("forceEnterLowpower lowpower_mode : %d\n", atoi(lowpower_mode));
                logPrint(MX_LOG_ERROR, "forceEnterLowpower lowpower_mode : %d\n", atoi(lowpower_mode));
                if(atoi(lowpower_mode) == 0)
                {
                    continue;
                }

                if(!access("/tmp/enter_shipmode", F_OK))
                {
                    SystemMode mode;
                    mode.system_mode = 1;
                    system("touch /tmp/tag_env_info_lock");
                    system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
                    sleep(3);
                    system_set_system_mode(&mode);
                    continue;
                }
            }
            else
            {
                logPrint(MX_LOG_ERROR, "getFWParaConfig get lowpower_mode failed\n");
                printf("forceEnterLowpower lowpower_mode is null\n");
            }
            system("touch /tmp/tag_env_info_lock");
            system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
            sleep(1);
            if (chdir("/usr/wifi/") != 0) 
            {
                continue;
            }

            int ret = system("./atbm_iot_cli rmmod");
            if (ret != 0) 
            {
                continue;
            }
        }
        
    }

	void CMCUSerialPortModule::DelayLowPowerRun()
	{
		while (1)
        {
			// printf("m_keyPress[%d], m_KeyCount[%d], m_netWakeUp[%d], m_NetCount[%d]\n",m_keyPress, m_KeyCount, m_netWakeUp, m_NetCount);
			logPrint(MX_LOG_INFOR, "m_keyPress[%d], m_KeyCount[%d], m_netWakeUp[%d], m_NetCount[%d]\n", m_keyPress, m_KeyCount, m_netWakeUp, m_NetCount);
			{
				std::unique_lock<std::mutex> lock(m_mutexDelayLowPower);
				if(m_keyPress)
				{
					m_KeyCount++;
				}

				if(m_netWakeUp)
				{
					m_NetCount++;
				}

				if(m_KeyCount >= 10)
				{
					m_keyPress = mxfalse;
				}
		
				if(m_NetCount >= 15)
				{
					if(access("/tmp/_netWakeUp",F_OK) == 0)
					{
						std::string strValue;
						linuxPopenExecCmd(strValue, "rm /tmp/_netWakeUp");
					}
					m_netWakeUp = mxfalse;
				}
			}
#ifdef	WIN32
            Sleep(100);
#else
            sleep(1);
#endif
		}
	}

	std::string CMCUSerialPortModule::sendAesMessageToMCU(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "get aes from ipc_manage:[%s]\r\n",strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (pJsonRoot == nullptr) {
			return procResult(std::string("500"), "", "Failed to parse JSON input");
		}

		cJSON *jsonValue = cJSON_GetObjectItem(pJsonRoot, "aesValue");
		if (jsonValue == nullptr) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "aesValue param parse failed!");
		}
		// printf("The base64 encoding of data：[%s]\n", (unsigned char*)jsonValue->valuestring);
		size_t decodedKeyLen = 0; 
		mbedtls_base64_decode(nullptr, 0, &decodedKeyLen, (const unsigned char*)jsonValue->valuestring, strlen(jsonValue->valuestring));
		unsigned char decodedKey[decodedKeyLen];
		memset(decodedKey, 0, sizeof(decodedKey));
		int ret = mbedtls_base64_decode( decodedKey, decodedKeyLen, &decodedKeyLen, (const unsigned char*)jsonValue->valuestring, 
			strlen(jsonValue->valuestring)); 
		if (ret != 0) { 
			logPrint(MX_LOG_INFOR, "Error: Base64 解码失败，错误码为:[%d]\r\n",ret);
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "","mbedtls_base64_decode faild!");
		} 

		// printf("解码后的内容: \r\n");
		// for (size_t i = 0; i < decodedKeyLen; i++)
		// {
		// 	printf("%d ", decodedKey[i]);
		// }
		// printf("\n");

		/// 只更新密钥到本地缓存，拿到ipcIndex再一起更新给MCU
		KeepaliveConfig config;
		memset(&config, 0, sizeof(config));
		config.has_aes_key = true;
		memcpy(config.aes_key, decodedKey, decodedKeyLen);
		config.has_ipc_index = false;
		config.ipc_index = 0;
		if(m_iaesFlag == 1)
		{
			
			m_iaesFlag = 0;
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("200"), "","get aes event success");
		}
		else
		{
			ipc_set_keepalive_config(&config);
			logPrint(MX_LOG_INFOR, "ipc set aes key wait mcu ack");
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "","get aes event failed");
		}
	}

	std::string CMCUSerialPortModule::setMcuLedStatus(std::string strParam)
    {
		logPrint(MX_LOG_INFOR, "get led_info from ipc_manage:[%s]\r\n",strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (pJsonRoot == nullptr) {
			return procResult(std::string("500"), "", "Failed to parse JSON input");
		}
		cJSON* led_Color = cJSON_GetObjectItem(pJsonRoot, "led_Color");
		cJSON* led_State = cJSON_GetObjectItem(pJsonRoot, "led_State");
		cJSON* on_time_ms = cJSON_GetObjectItem(pJsonRoot, "on_time_ms");
		cJSON* off_time_ms = cJSON_GetObjectItem(pJsonRoot, "off_time_ms");

		if (led_Color == nullptr || !cJSON_IsNumber(led_Color)) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "led_color param parse failed!");
		}
		if (led_State == nullptr || !cJSON_IsNumber(led_State)) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "led_state param parse failed!");
		}
		if (on_time_ms == nullptr || !cJSON_IsNumber(on_time_ms)) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "on_time_ms param parse failed!");
		}
		if (off_time_ms == nullptr || !cJSON_IsNumber(off_time_ms)) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "off_time_ms param parse failed!");
		}

		int ledColor = led_Color->valueint;
		int ledState =	led_State->valueint;
		int onTimeMs =	on_time_ms->valueint;
		int offTimeMs =	off_time_ms->valueint;

		cJSON_Delete(pJsonRoot);

		logPrint(MX_LOG_INFOR, "get led_info is:[%d]-[%d]-[%d]-[%d]\r\n", ledColor, ledState, onTimeMs, offTimeMs);

     	Ipc ipc = Ipc_init_default;
        IotPacket packet = IotPacket_init_zero;
        LedInfo led_info;
        size_t dataBufLen = 0;

		led_info.led_color = (LedInfo_Color)ledColor;
        led_info.led_state = (LedInfo_State)ledState;

        led_info.has_led_brightness = true;
        led_info.led_brightness = 255;

		led_info.has_led_flashing_freq = true;
        led_info.led_flashing_freq.on_time_ms = onTimeMs;
		led_info.led_flashing_freq.off_time_ms = offTimeMs;

        packet.which_payload = IotPacket_ipc_tag;
        packet.payload.ipc.which_payload = Ipc_led_info_tag;
        memcpy(&packet.payload.ipc.payload.led_info, &led_info, sizeof(LedInfo));
        pb_get_encoded_size(&dataBufLen, IotPacket_fields, &packet);

		logPrint(MX_LOG_INFOR, "dataBufLen:[%d]!\r\n", dataBufLen);

		uint8_t *dataBuf = (uint8_t *)malloc(dataBufLen);
        if (dataBuf == NULL) {
            return procResult(std::string("500"), "", "LedSet dataBuf malloc failed!");
        }
        memset(dataBuf, 0, dataBufLen);

        ipc.which_payload = Ipc_led_info_tag;
        memcpy(&ipc.payload.led_info, &led_info, sizeof(LedInfo));

        mx_encode_iot_ipc_proto(Ipc_IpcID_SET_LED_INFO, &ipc, dataBuf, dataBufLen);
		if(m_iledState)
		{
			 mx_proto_send(dataBuf, dataBufLen);
		}

        // 现在程序设置led没有返回消息

        free(dataBuf);
        return procResult(std::string("200"), "","set mcu_led success");
    }

	std::string CMCUSerialPortModule::setMcuPirStatus(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "get pir_info from ipc_manage:[%s]\r\n",strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (pJsonRoot == nullptr) {
			return procResult(std::string("500"), "", "Failed to parse JSON input");
		}
		cJSON* pir_Status = cJSON_GetObjectItem(pJsonRoot, "pir_Status");
		cJSON* time_Interval = cJSON_GetObjectItem(pJsonRoot, "pir_Interval");
		cJSON* sensitivity = cJSON_GetObjectItem(pJsonRoot, "pir_Sensitivity");

		if((pir_Status == nullptr || !cJSON_IsNumber(pir_Status)) && 
			(time_Interval == nullptr || !cJSON_IsNumber(time_Interval)) && 
			(sensitivity == nullptr || !cJSON_IsNumber(sensitivity)))
		{
			return procResult(std::string("500"), "", "pir_info param parse failed!");
		}

		int pirStatus =  pir_Status ? pir_Status->valueint : 1;
		int pirTimeInterval= time_Interval ? time_Interval->valueint : 1;
		int pirSensitivity = sensitivity ? sensitivity->valueint : 3;

		cJSON_Delete(pJsonRoot);

		logPrint(MX_LOG_INFOR, "get pir_info is:[%d]-[%d]-[%d]\r\n", pirStatus, pirTimeInterval, pirSensitivity);

		Ipc ipc = Ipc_init_default;
        IotPacket packet = IotPacket_init_zero;
        PirInfo pir_info;
        size_t dataBufLen = 0;

		pir_info.has_switcher = true;
		pir_info.switcher = (OptionalSwitcher)pirStatus;
		pir_info.has_sensitivity = true;
		pir_info.sensitivity = (PirInfo_Sensitivity)pirSensitivity;
		pir_info.has_wakeup_cooling_time = true;
		pir_info.wakeup_cooling_time = pirTimeInterval;

		packet.which_payload = IotPacket_ipc_tag;
        packet.payload.ipc.which_payload = Ipc_pir_info_tag;
        memcpy(&packet.payload.ipc.payload.pir_info, &pir_info, sizeof(PirInfo));
        pb_get_encoded_size(&dataBufLen, IotPacket_fields, &packet);

		logPrint(MX_LOG_INFOR, "dataBufLen:[%d]!\r\n", dataBufLen);

		uint8_t *dataBuf = (uint8_t *)malloc(dataBufLen);
        if (dataBuf == NULL) {
            return procResult(std::string("500"), "", "PirSet dataBuf malloc failed!");
        }
        memset(dataBuf, 0, dataBufLen);

        ipc.which_payload = Ipc_pir_info_tag;
        memcpy(&ipc.payload.pir_info, &pir_info, sizeof(PirInfo));

        mx_encode_iot_ipc_proto(Ipc_IpcID_SET_PIR_INFO, &ipc, dataBuf, dataBufLen);
        mx_proto_send(dataBuf, dataBufLen);

		free(dataBuf);
		return procResult(std::string("200"), "","set mcu_pir status success");
	}

	std::string CMCUSerialPortModule::setSoundAndLightAlarm(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "get SoundAndLightAlarm info from ipc_manage:[%s]\r\n",strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (pJsonRoot == nullptr) {
			return procResult(std::string("500"), "", "Failed to parse JSON input");
		}

		cJSON* alarm_Status = cJSON_GetObjectItem(pJsonRoot, "status");
		if (alarm_Status == nullptr || !cJSON_IsNumber(alarm_Status)) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "SoundAndLightAlarm alarm_Status param parse failed!");
		}

		int alarmStatus = alarm_Status->valueint;

		cJSON_Delete(pJsonRoot);
		logPrint(MX_LOG_INFOR, "get SoundAndLightAlarm status is:[%d]\r\n", alarmStatus);
		//开启白光灯 并播放报警音频30s
		if(alarmStatus == 1)
		{
			//声音警告
			if(!playAudioFile(PLAY_FILE_WARMING_ALARM,4,30))
			{
				return procResult(std::string("500"), "", "SoundAndLightAlarm playAudioFile warming_alarm  failed!");
			}
			//打开白光灯
			if(!setAlarmLights(alarmStatus))
			{
				return procResult(std::string("500"), "", "SoundAndLightAlarm setAlarmLights lights_alarm  failed!");
			}

			return procResult(std::string("200"), "","set SoundAndLightAlarm status success");
		}else if(alarmStatus == 0)
		{
			//关闭警告音
			if(!stopAudioFile(PLAY_FILE_WARMING_ALARM))
			{
				return procResult(std::string("500"), "", "SoundAndLightAlarm playAudioFile warming_alarm  failed!");
			}

			//关闭白光灯
			if(!setAlarmLights(alarmStatus))
			{
				return procResult(std::string("500"), "", "SoundAndLightAlarm setAlarmLights lights_alarm  failed!");
			}

			return procResult(std::string("200"), "","set SoundAndLightAlarm status success");
		}else
		{
			return procResult(std::string("500"), "", "alarmStatus input status error!");
		}

	}

	std::string CMCUSerialPortModule::getBatteryChargingState()
	{
		system_device_status_request();

		bool bRunning = true;
		while (bRunning)
		{
			std::unique_lock<std::mutex> lock(m_mutexSendBatteryInfo);
			auto status = m_conditionBatteryInfo.wait_for(lock, std::chrono::seconds(1));
			if (status == std::cv_status::timeout) {
				// 等待超时，返回错误
				return procResult(std::string("500"), "", "get batteryChargingState status error!");
			}

			cJSON *jsonRoot = cJSON_CreateObject();
			cJSON *jsonParam = cJSON_CreateObject();

			cJSON_AddStringToObject(jsonRoot, "event", "batteryChargingState");
			cJSON_AddNumberToObject(jsonParam, "status", m_ichargeStatus);
			cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

			char *out = cJSON_Print(jsonRoot);
			std::string strResult = std::string(out);
			cJSON_Delete(jsonRoot);
			if (out)
				free(out);
			bRunning = false;
			return strResult;
		}
		return procResult(std::string("400"), "", "get batteryChargingState runing error!");
	}

	std::string CMCUSerialPortModule::getBatteryCapacity()
	{
		system_device_status_request();

		bool bRunning = true;
		while (bRunning)
		{
			std::unique_lock<std::mutex> lock(m_mutexSendBatteryInfo);
			auto status = m_conditionBatteryInfo.wait_for(lock, std::chrono::seconds(1));
			if (status == std::cv_status::timeout) {
				// 等待超时，返回错误
				return procResult(std::string("500"), "", "get batteryCapacity value error!");
			}

			cJSON *jsonRoot = cJSON_CreateObject();
			cJSON *jsonParam = cJSON_CreateObject();

			cJSON_AddStringToObject(jsonRoot, "event", "batteryCapacity");
			cJSON_AddNumberToObject(jsonParam, "capacityValue", m_iCapacity);
			cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

			char *out = cJSON_Print(jsonRoot);
			std::string strResult = std::string(out);
			cJSON_Delete(jsonRoot);
			if (out)
				free(out);
			bRunning = false;
			return strResult;			
		}
		return procResult(std::string("400"), "", "get batteryCapacity runing error!");
	}

	std::string CMCUSerialPortModule::sendSpeakerVoice(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "get voice command from ipc_manage:[%s]\r\n", strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (pJsonRoot == nullptr) {
			return procResult(std::string("500"), "", "Failed to parse JSON input");
		}

		cJSON *jsonCommand = cJSON_GetObjectItem(pJsonRoot, "command");
		if (jsonCommand == nullptr) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "command param parse failed!");
		}
		std::string command = std::string(jsonCommand->valuestring);

		cJSON_Delete(pJsonRoot);

		if(!playAudioFile(command.c_str(),4,0))
		{
			return procResult(std::string("500"), "","playAudioFile command faild!");
		}

		return procResult(std::string("200"), "","send voice command success");
	}

    std::string CMCUSerialPortModule::setLogManageOnlineState(std::string strParam)
    {
        // mxbool isOnline = mxfalse;
        std::string errCode;
        std::string errMsg;
        cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
        if(pJsonRoot)
        {
            cJSON *pOnlineState = cJSON_GetObjectItem(pJsonRoot,"onlineState");
            if(pOnlineState)
            {
                errCode = "200";
                errMsg = "success";
                if(cJSON_IsTrue(pOnlineState) == 1)
                {
                    // isOnline = mxtrue;
                }
                else
                {
                    // isOnline = mxfalse;
                }
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
        }
        else
        {
            errCode = "404";
            errMsg = "Params err";
        }
        if(errCode.compare("200") == 0)
        {
            //TODO
            //do something
        }
        return procResult(errCode, errMsg,
            "mcu set log online success");        
    }

	mxbool CMCUSerialPortModule::makeToReset()
	{
		playAudioFile(PLAY_FILE_RESET_SUCCESS,4,0);

		std::string strValue;
		linuxPopenExecCmd(strValue, "cd /usr/wifi ; ./atbm_iot_cli wifi_mode AP");

		int ret = system("fw_setenv -c user");
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "Error executing command: fw_printenv fw_setenv -c user\n");
			return mxfalse;
		}

		ret = system("fw_printenv user-backup > 1");
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "Error executing command: fw_printenv user-backup\n");
			return mxfalse;
		}

		ret = system("fw_setenv -s 1 user");
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "Error executing command: fw_setenv -s 1 user\n");
			return mxfalse;
		}

		ret = system("fw_setenv user camera_switch 1");
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "Error executing command: fw_setenv -s 1 user\n");
			return mxfalse;
		}

		HostEvent host_event;
		host_event.type = HostEvent_Type_BIND_REMOVE;
		ipc_set_host_event(&host_event);
		/// 重置ipc ，重启使ap改变
		std::string strResetValue;
		linuxPopenExecCmd(strResetValue, "tag_env_info --set HW 70mai_dn_sw 1");
		linuxPopenExecCmd(strResetValue, "tag_env_info --set HW wl_mode off");
		linuxPopenExecCmd(strResetValue, "tag_env_info --set HW ir_mode auto");
		linuxPopenExecCmd(strResetValue, "tag_env_info --set HW pwm_duty 1900:3100");
		
		linuxPopenExecCmd(strResetValue, "tag_env_info --set HW reset_value 1");

		system("touch /tmp/tag_env_info_lock");
		system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
		sleep(3);
		SendFactoryCmd("AT+REBOOT");
		
		return mxtrue;
	}

	std::string CMCUSerialPortModule::setMcuLedSwitch(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "get led_switch from ipc_manage:[%s]\r\n",strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (pJsonRoot == nullptr) {
			return procResult(std::string("500"), "", "Failed to parse JSON input");
		}
		cJSON* led_State = cJSON_GetObjectItem(pJsonRoot, "LedSwitch");

		if (led_State == nullptr || !cJSON_IsNumber(led_State)) {
			cJSON_Delete(pJsonRoot);
			return procResult(std::string("500"), "", "led_switch param parse failed!");
		}
	
		m_iledState =	led_State->valueint;

		cJSON_Delete(pJsonRoot);

		logPrint(MX_LOG_INFOR, "get led_switch is:[%d]\r\n", m_iledState);

     	// Ipc ipc = Ipc_init_default;
        // IotPacket packet = IotPacket_init_zero;
        // LedInfo led_info;
        // size_t dataBufLen = 0;

		// led_info.led_color = (LedInfo_Color)4;
        // led_info.led_state = (LedInfo_State)ledState;

        // led_info.has_led_brightness = false;
        // led_info.led_brightness = 255;

		// led_info.has_led_flashing_freq = false;
        // led_info.led_flashing_freq.on_time_ms = 1;
		// led_info.led_flashing_freq.off_time_ms = 4;

        // packet.which_payload = IotPacket_ipc_tag;
        // packet.payload.ipc.which_payload = Ipc_led_info_tag;
        // memcpy(&packet.payload.ipc.payload.led_info, &led_info, sizeof(LedInfo));
        // pb_get_encoded_size(&dataBufLen, IotPacket_fields, &packet);

		// logPrint(MX_LOG_INFOR, "dataBufLen:[%d]!\r\n", dataBufLen);

		// uint8_t *dataBuf = (uint8_t *)malloc(dataBufLen);
        // if (dataBuf == NULL) {
        //     return procResult(std::string("500"), "", "LedSet dataBuf malloc failed!");
        // }
        // memset(dataBuf, 0, dataBufLen);

        // ipc.which_payload = Ipc_led_info_tag;
        // memcpy(&ipc.payload.led_info, &led_info, sizeof(LedInfo));

        // mx_encode_iot_ipc_proto(Ipc_IpcID_SET_LED_INFO, &ipc, dataBuf, dataBufLen);
        // mx_proto_send(dataBuf, dataBufLen);

        // free(dataBuf);
        return procResult(std::string("200"), "","set mcu_led status success");
	}

	mxbool CMCUSerialPortModule::setAlarmLights(int ledState)
	{
		std::string cmd;
		if(ledState == 1)
		{
			cmd = "wled on";
		}
		else if(ledState == 0)
		{
			cmd = "wled off";
		}
		 
		int ret = system(cmd.c_str());
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "Error executing command to setAlarmLights\n");
			return mxfalse;
		}
		return mxtrue;
	}

	int CMCUSerialPortModule::SendFactoryCmd(std::string strCommand)
    {
        CmdInfo_Request info;
        int ret = 0;
        info.data = (pb_bytes_array_t *)malloc(sizeof(pb_bytes_array_t) + sizeof(pb_byte_t)*(strCommand.length() + 1));
        if(info.data == NULL)
        {
            ret = -1;
        }

		memcpy(info.data->bytes, strCommand.c_str(), strCommand.length());
        info.data->bytes[strCommand.length()] = 0;
		info.data->size = strCommand.length() + 1;

        factory_cmd_info_request(&info);
        free(info.data);
        return ret;
    }

	int CMCUSerialPortModule::FactoryReset()
    {
		int fpid = fork();
		if (fpid < 0)
			system("reboot");
		else if (fpid == 0)
		{
			system("insmod_uvc");
			system("uvc-gadget 0");
		}
		sleep(3);
		system("enter_factory_auth");
		std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW 70mai_factory_mode");

		int iPos = strCmdValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_factory_mode");
			system("reboot");			
		}

		std::string strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		if(atoi(strValue.c_str()) == 1)
        	SendFactoryCmd("AT+SET_FACT_MODE 2");

		system("reboot");
        return 0;
    }

	std::string CMCUSerialPortModule::sendDevMsg(std::string strMsg)
	{
		CModule *module = (CModule *)this;
		std::string strRet = module->output(m_strDevManageGUID, m_strDevManageServer, (unsigned char *)strMsg.c_str(),
								strMsg.length());
		
		return strRet;
	}

}


