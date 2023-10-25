#include "event_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "event_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "log_mx.h"
#include "fw_env_para.h"
#include "common.h"

namespace maix {
	CEventManageModule::CEventManageModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
		, m_objEventManageAttemptReport(NULL)
	{
	}

	CEventManageModule::~CEventManageModule()
	{
	}

	mxbool CEventManageModule::init()
	{
		if (!initConnectModule())
		{
			std::cout << "[EventManageApp]: init connect module failed" << std::endl;
			return mxfalse;
		}
			
		if (!initServer())
		{
			std::cout << "[EventManageApp]: init server failed" << std::endl;
			return mxfalse;
		}
			
		if (!initLowPower())
		{
			std::cout << "[EventManageApp]: init lower power failed" << std::endl;
			return mxfalse;
		}
			

		if (!initAttemptReport())
		{
			std::cout << "[EventManageApp]: init attempt report failed" << std::endl;
			return mxfalse;
		}
			
        if(!initMcuRemote())
        {
        	std::cout << "[EventManageApp]: init mcu remote failed" << std::endl;
            return mxfalse;
        }
		
		if(!initIpcEventServer())
			return mxfalse;

		return mxtrue;
	}

	mxbool CEventManageModule::unInit()
	{
		return mxbool();
	}

	mxbool CEventManageModule::initServer()
	{
		int iComServerConfigNum = 0;
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

				if (strComType.compare("RCF_LOCAL") == 0)
				{
					std::shared_ptr<CComClientEndPoint> comClientEndPoint(
						new CComClientEndPoint(this));

					if (strName.compare("local_event_server") != 0)
					{

					}

				}
				else if (strComType.compare("RCF_EVENT") == 0)
				{
					int iType = 0;
					if (!getConfig(acComServerConfig, "TYPE", iType))
					{
						return mxfalse;
					}

					std::string strIP;
					if (!getConfig(acComServerConfig, "IP", strIP))
					{
						return mxfalse;
					}

					int iPort = 0;
					if (!getConfig(acComServerConfig, "PORT", iPort))
					{
						return mxfalse;
					}

					std::string strUnix;
					if (!getConfig(acComServerConfig, "UNIX", strUnix))
					{
						return mxfalse;
					}

					int iLen = 0;
					if (!getConfig(acComServerConfig, "LEN", iLen))
					{
						return mxfalse;
					}

					if (strName.compare("remote_event_server") == 0)
					{
						CEventManageRemoteEventServer * objEventManageRemoteEventServer =
							new CEventManageRemoteEventServer(this);

						if (!objEventManageRemoteEventServer)
							return mxfalse;

						objEventManageRemoteEventServer->init();

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
							objEventManageRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CEventManageModule::initConnectModule()
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
			int iComServerConfigNum = 0;
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
						int iType = 0;
						if (!module->getConfig(acComServerConfig, "TYPE", iType))
						{
							return mxfalse;
						}
						else
						{
							if (iType == 5)
								iType = 4;
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

						int iPort = 0;
						if (!module->getConfig(acComServerConfig, "PORT", iPort))
						{
							return mxfalse;
						}

						std::string strUnix;
						if (!module->getConfig(acComServerConfig, "UNIX", strUnix))
						{
							return mxfalse;
						}

						int iLen = 0;
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

	mxbool CEventManageModule::initLowPower()
	{
		if (!getConfig("LOW_POWER_REMOTE_EVENT", "GUID",
			m_strLowPowerGUID))
		{
			return mxfalse;
		}

		if (!getConfig("LOW_POWER_REMOTE_EVENT", "SERVER",
			m_strLowPowerServer))
		{
			return mxfalse;
		}

		m_threadEnterLowPower = std::thread([this]() {
			this->lowPowerRun();
		});

		return mxtrue;
	}

	mxbool CEventManageModule::initAttemptReport()
	{
		std::shared_ptr<CEventManageAttemptReport>
			objEventManageAttemptReport = std::make_shared<CEventManageAttemptReport>(this);
		if (NULL == objEventManageAttemptReport)
			return mxfalse;

		if (!objEventManageAttemptReport->init())
			return mxfalse;

		m_threadEventManageAttemptReport = std::thread([objEventManageAttemptReport]() {
			objEventManageAttemptReport->run();
		});

		m_objEventManageAttemptReport = objEventManageAttemptReport;

		return mxtrue;
	}

    mxbool CEventManageModule::initMcuRemote()
    {
        if (!getConfig("MCU_MANAGE_REMOTE_EVENT", "GUID",
            m_strMcuManageGUID))
        {
            return mxfalse;
        }

        if (!getConfig("MCU_MANAGE_REMOTE_EVENT", "SERVER",
            m_strMcuManageServer))
        {
            return mxfalse;
        }
        return mxtrue;
    }

	void CEventManageModule::lowPowerRun()
	{
		while (1)
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.wait(lock);
			std::string procResult = lowPowerProc();
			std::string strResult = output(m_strLowPowerGUID,
				m_strLowPowerServer,
				(unsigned char*)procResult.c_str(), procResult.length());

#ifdef _WIN32
			Sleep(1000);
#else
			usleep(1000 * 1000);
#endif
		}
	}

	std::string CEventManageModule::lowPowerProc()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EventManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

	std::string CEventManageModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "event remote event:%s", strEvent.c_str());
		if (0 == strEvent.compare("EnterLowPower"))
		{
			return enterLowPower(strParam);
		}
		else if (0 == strEvent.compare("AlarmEvent"))
		{
			return sendAlarmEvent(strEvent, strParam);
		}
        else if (0 == strEvent.compare("MCUEvent"))
        {
            return parseMCUEvemt(strParam);
        }
		else if(0 == strEvent.compare("sendSpecData"))
		{
			return sendSpecData(strEvent, strParam);
		}
		else if(0 == strEvent.compare("AlertEvent"))
		{
			return sendAlertEvent(strEvent, strParam);
		}
		else if (0 == strEvent.compare("CenterEvent"))
		{
			sendToCenterEventOccurs();
			return std::string("ok");
		}
		else if (0 == strEvent.compare("CenterNoEvent"))
		{
			sendToCenterEventNoOccurs();
			return std::string("ok");
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	std::string CEventManageModule::sendSpecData(std::string strEvent, std::string strParam)
	{
		cJSON* jsonRoot = cJSON_CreateObject();
		cJSON *pJsonParam = cJSON_Parse(strParam.c_str());
		if(pJsonParam)
		{
			cJSON_AddStringToObject(jsonRoot, "event", strEvent.c_str());
			cJSON_AddItemToObject(jsonRoot, "param" ,pJsonParam);
		}
		char* out = cJSON_Print(jsonRoot);

		cJSON_free(jsonRoot);
		jsonRoot = NULL;

		return output(m_strIpcManageGUID, m_strIpcManageServer, (unsigned char*)out, strlen(out) + 1);
	}

	mxbool CEventManageModule::sendSpecDataVoiceLight(std::string strParam)
	{
		std::string strValue;
		int iValueAlarm= atoi(getFWParaConfig("auto_audible_visual_alarm"));
		if(iValueAlarm == 1)
		{
			cJSON *jsonParam = cJSON_Parse(strParam.c_str());
			cJSON *jsonValue = cJSON_GetObjectItem(jsonParam, "value");
			if(jsonValue)
			{
				strValue = jsonValue->valuestring;
			}

			cJSON* jsonParamData = cJSON_CreateObject();
			cJSON_AddStringToObject(jsonParamData, "configName", "VoiceLightWarning");
			cJSON_AddStringToObject(jsonParamData, "configValue", strValue.c_str());
			std::string strData = cJSON_Print(jsonParamData);

			std::string strResult = sendSpecData("sendSpecData", strData);
			if(jsonParam)
			{
				cJSON_free(jsonParam);
				jsonParam = NULL;
			}
		}
		return mxtrue;
	}

	mxbool CEventManageModule::sendSpecDataRecord()
	{
		cJSON* jsonParamData = cJSON_CreateObject();
		std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW 70mai_dn_sw");
		int iPos = strCmdValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_dn_sw");
			return mxfalse;
		}
		std::string strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		int iValue = atoi(strValue.c_str());
		if(iValue == 0)
		{
			cJSON_AddStringToObject(jsonParamData, "configName", "RedLightRecord");
		}
		else if(iValue == 1)
		{
			cJSON_AddStringToObject(jsonParamData, "configName", "WhiteLightRecord");
		}
		else if(iValue == 2)
		{
			cJSON_AddStringToObject(jsonParamData, "configName", "NoLightRecord");
		}
		else
		{
			logPrint(MX_LOG_ERROR, "the 70mai_dn_sw is invalued");
		}
		std::string strData = cJSON_Print(jsonParamData);
		std::string strResult = sendSpecData("sendSpecData", strData);
		if(jsonParamData)
		{
			cJSON_free(jsonParamData);
			jsonParamData = NULL;
		}
		return mxtrue;
	}

	std::string CEventManageModule::procResult(std::string code,
		std::string strMsg, std::string strErr)
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

	std::string CEventManageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		return procResult(std::string("200"), "",
			"event manage enter low power");
	}

	std::string CEventManageModule::sendAlarmEvent(std::string strEvent,
		std::string strParam)
	{
		if (m_objEventManageAttemptReport)
		{
			cJSON *jsonRoot = cJSON_CreateObject();
			cJSON *jsonParam = cJSON_Parse(strParam.c_str());

			cJSON_AddStringToObject(jsonRoot, "event", strEvent.c_str());
			cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

			char *out = cJSON_Print(jsonRoot);
			std::string strEventConfig = std::string(out);
			cJSON_Delete(jsonRoot);
			if (out)
				free(out);

			T_EventMessage tEventMessage;
			tEventMessage.iTimeOut = getCurrentTime() + 15;
			tEventMessage.iTryNum = EVENT_TRY_NUM;
			tEventMessage.strData = strEventConfig;

			if (m_objEventManageAttemptReport->pushEventData(tEventMessage))
			{
				sendSpecDataRecord();
				sendSpecDataVoiceLight(strParam);
				// sendToCenterEventOccurs();
				return std::string("ok");
			}
			else 
			{
				return std::string("error");
			}
		}
		else
		{
			return std::string("error");
		}
		
	}

	std::string CEventManageModule::sendAlertEvent(std::string strEvent, std::string strParam)
	{
		if (m_objEventManageAttemptReport)
		{
			cJSON *jsonRoot = cJSON_CreateObject();
			cJSON *jsonParam = cJSON_Parse(strParam.c_str());

			cJSON_AddStringToObject(jsonRoot, "event", strEvent.c_str());
			cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

			char *out = cJSON_Print(jsonRoot);
			std::string strEventConfig = std::string(out);
			cJSON_Delete(jsonRoot);
			if (out)
			{
				free(out);
				out = NULL;
			}

			T_EventMessage tEventMessage;
			tEventMessage.iTimeOut = getCurrentTime() + 15;
			tEventMessage.iTryNum = EVENT_TRY_NUM;
			tEventMessage.strData = strEventConfig;

			if (m_objEventManageAttemptReport->pushEventData(tEventMessage))
			{
				return procResult(std::string("200"), "", "ok");
			}
			else 
			{
				return procResult(std::string("500"), "", "push event error");
			}
		}
		else
		{
			return procResult(std::string("500"), "", "report obj null");
		}
	}

	void CEventManageModule::sendToCenterEventOccurs()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "eventOccurs");
		cJSON_AddBoolToObject(jsonParam,"occursState",mxtrue);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);
         
        output(m_strLowPowerGUID,m_strLowPowerServer,(unsigned char*) strResult.c_str(),strResult.length());
	}

	void CEventManageModule::sendToCenterEventNoOccurs()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "eventNoOccurs");
		cJSON_AddBoolToObject(jsonParam,"eventState",mxfalse);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);
         
        output(m_strLowPowerGUID,m_strLowPowerServer,(unsigned char*) strResult.c_str(),strResult.length());
	}

    std::string CEventManageModule::parseMCUEvemt(std::string strParam)
    {
        std::string errCode;
        std::string errMsg;

        std::string eventType;
        std::string eventValue;

        cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
        if(pJsonRoot)
        {
            cJSON *pEventType = cJSON_GetObjectItem(pJsonRoot,"eventType");
            if(pEventType)
            {
                errCode = "200";
                errMsg = "success";
                eventType = pEventType->valuestring;
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
            cJSON *pEventVaule = cJSON_GetObjectItem(pJsonRoot,"value");
            if(pEventVaule)
            {
                errCode = "200";
                errMsg = "success";
                eventValue = pEventVaule->valuestring;
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
            if(0 == eventType.compare("PirEvent"))
            {
                //pir事件
				cJSON* jsonParam = cJSON_CreateObject();
				cJSON_AddStringToObject(jsonParam, "configName", "PirTouch");
				cJSON_AddStringToObject(jsonParam, "configValue", "1");
				std::string strData = cJSON_Print(jsonParam);
				std::string strResult = sendSpecData("sendSpecData", strData);

				if(jsonParam)
				{
					cJSON_free(jsonParam);
					jsonParam = NULL;
				}
                logPrint(MX_LOG_INFOR, "receive pir event");
				time_t currentTime = time(NULL);
				char* pCount = getFWParaConfig("pir_false_wakeup_count");
				if(pCount == NULL)
				{
					setFWParaConfig("pir_false_wakeup_count", "0", 1);
				}
				else
				{
					int iCount = atoi(pCount);
					iCount ++;
					
					char *pOldTime = getFWParaConfig("pir_wakeup_time");
					if(pOldTime != NULL)
					{
						time_t iOldTime = (time_t)atoi(pOldTime);
						if(currentTime - iOldTime > 10 * 60)
						{
							setFWParaConfig("pir_false_wakeup_count", "0", 1);
						}
						else
						{
							setFWParaConfig("pir_false_wakeup_count", std::to_string(iCount).c_str(), 1);
						}
					}
				}
				setFWParaConfig("pir_wakeup_time", std::to_string(currentTime).c_str(), 1);
            }
            else if(0 == eventType.compare("NetEvent"))
            {
                //网络唤醒事件
                logPrint(MX_LOG_INFOR, "receive net event");
            }
        }

        errCode = "200";
        errMsg = "success";
        return procResult(errCode, errMsg,
            "mcu event success");   
    }

	mxbool CEventManageModule::initIpcEventServer()
	{
		if(!getConfig("IPC_MANAGE_REMOTE_EVENT", "GUID", m_strIpcManageGUID))
		{
			return mxfalse;
		}
		if(!getConfig("IPC_MANAGE_REMOTE_EVENT", "SERVER", m_strIpcManageServer))
		{
			return mxfalse;
		}
		return mxtrue;
	}

	int64_t CEventManageModule::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}
}