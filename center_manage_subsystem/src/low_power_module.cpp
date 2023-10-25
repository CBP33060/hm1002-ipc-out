#include "low_power_module.h"
#include "com_rpc_server_end_point.h"
#include "low_power_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#include "log_mx.h"
#include "sys/prctl.h"

namespace maix {
	CLowPowerModule::CLowPowerModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
		, m_eLastEventDetectType(E_EventDetectStart)
		, m_eEventDetectType(E_EventDetectStart)
		, m_iCumulativeTime(0)
		, m_iEventDetectTime(10)
		, m_iRecordLastTime(30)
		, m_iCumulativeRecordTime(0)
		, m_iLongLastTime(60)
        ,m_iCumulativeLongLastTime(0)
        ,m_iUserSetRecordTime(30)
	{
	}

	CLowPowerModule::~CLowPowerModule()
	{
	}

	mxbool CLowPowerModule::init()
	{
		if (!initConnectModule())
		{
			std::cout << "[CenterManageApp]: init connect module failed" << std::endl;
			return mxfalse;
		}
			

		if (!initServer())
		{
			std::cout << "[CenterManageApp]: init server failed" << std::endl;
			return mxfalse;
		}
			

		if (!getConfig("TIME_CONFIG", "EVENT_DETECT_TIMEOUT", 
			m_iEventDetectTime))
		{
			std::cout << "[CenterManageApp]: get time config event detect timeout" << std::endl;
			return mxfalse;
		}

		if (!getConfig("TIME_CONFIG", "EVENT_RECORD_TIMEOUT",
			m_iRecordLastTime))
		{
			std::cout << "[CenterManageApp]: get time config event record timeout" << std::endl;
			return mxfalse;
		}
        m_iUserSetRecordTime = m_iRecordLastTime;

		if (!getConfig("TIME_CONFIG", "LONG_LAST_TIMEOUT",
			m_iLongLastTime))
		{
			std::cout << "[CenterManageApp]: get time config long last timeout" << std::endl;
			return mxfalse;
		}

		if (!initEventDetect())
		{
			std::cout << "[CenterManageApp]: init event detect failed" << std::endl;
			return mxfalse;
		}

		if (!initLowPowerFlow())
		{
			std::cout << "[CenterManageApp]: init low power flow failed" << std::endl;
			return mxfalse;
		}
			
		return mxtrue;
	}

	mxbool CLowPowerModule::unInit()
	{
		return mxbool();
	}

	mxbool CLowPowerModule::initServer()
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

				if (strComType.compare("RCF_EVENT") == 0)
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
						CLowPowerRemoteEventServer * objLowPowerRemoteEventServer =
							new CLowPowerRemoteEventServer(this);

						if (!objLowPowerRemoteEventServer)
							return mxfalse;

						objLowPowerRemoteEventServer->init();

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
							objLowPowerRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CLowPowerModule::initConnectModule()
	{
		int iConnectModuleNum = 0;
		if (!getConfig("CONNECT_MODULE", "NUM", iConnectModuleNum))
		{
			std::cout << "[CenterManageApp]: get connect module num failed" << std::endl;
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
				std::cout << "[CenterManageApp]: get connect module name failed" << std::endl;
				return mxfalse;
			}

			std::string  strGUID;
			if (!getConfig(acConnectModule, "GUID", strGUID))
			{
				std::cout << "[CenterManageApp]: get connect module guid failed" << std::endl;
				return mxfalse;
			}

			std::string  strConfig;
			if (!getConfig(acConnectModule, "CONFIG", strConfig))
			{
				std::cout << "[CenterManageApp]: get connect module config failed" << std::endl;
				return mxfalse;
			}

			std::shared_ptr<CIModule> imodule = NULL;
			if (!getConnectModule(strGUID, imodule))
			{
				std::shared_ptr<CModule> newModule(
					new CModule(strGUID, strName));

				if (!newModule->loadConfig(strConfig))
				{
					std::cout << "[CenterManageApp]: new module load config failed" << std::endl;
					return mxfalse;
				}

				imodule = newModule;
				if (!connect(imodule))
				{
					std::cout << "[CenterManageApp]: connect config failed" << std::endl;
					return mxfalse;
				}
					
			}

			CModule *module = dynamic_cast<CModule *>(imodule.get());
			int iComServerConfigNum;
			if (!module->getConfig("COM_SERVER_CONFIG", "NUM", iComServerConfigNum))
			{
				std::cout << "[CenterManageApp]: get com server config num failed" << std::endl;
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
					std::cout << "[CenterManageApp]: get com server config name failed" << std::endl;
					return mxfalse;
				}

				if (!module->isClientExist(strName))
				{
					std::string strComType;
					if (!module->getConfig(acComServerConfig, "COM_TYPE", strComType))
					{
						std::cout << "[CenterManageApp]: get com server config com type failed" << std::endl;
						return mxfalse;
					}

					if (strComType.compare("RCF_EVENT") == 0)
					{
						int iType = 0;
						if (!module->getConfig(acComServerConfig, "TYPE", iType))
						{
							std::cout << "[CenterManageApp]: get com server config type failed" << std::endl;
							return mxfalse;
						}
						else
						{
							if (iType == 5)
							{
								iType = 4;
							}
						}

						std::string strIP;
						if (!module->getConfig(acComServerConfig, "IP", strIP))
						{
							std::cout << "[CenterManageApp]: get com server config ip failed" << std::endl;
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
							std::cout << "[CenterManageApp]: get com server config port failed" << std::endl;
							return mxfalse;
						}

						std::string strUnix;
						if (!module->getConfig(acComServerConfig, "UNIX", strUnix))
						{
							std::cout << "[CenterManageApp]: get com server config unix failed" << std::endl;
							return mxfalse;
						}

						int iLen = 0;
						if (!module->getConfig(acComServerConfig, "LEN", iLen))
						{
							std::cout << "[CenterManageApp]: get com server config len failed" << std::endl;
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
						{
							std::cout << "[CenterManageApp]: reg client failed" << std::endl;
							return mxfalse;
						}
					}

				}
			}

		}

		return mxtrue;
	}

	mxbool CLowPowerModule::initEventDetect()
	{
		m_threadEventDetect = std::thread([this]() {
			this->eventDetectProc();
		});

		return mxtrue;
	}

	mxbool CLowPowerModule::initLowPowerFlow()
	{
		std::shared_ptr<CLowPowerInformFlow> lowPowerInformFlow(
			new CLowPowerInformFlow(this));
		if (!lowPowerInformFlow->init())
			return mxfalse;

		m_lowPowerInformFlow = lowPowerInformFlow;
		m_threadLowPowerFlow = std::thread([lowPowerInformFlow]() {
			lowPowerInformFlow->run();
		});

		return mxtrue;
	}

	std::string CLowPowerModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		if (0 == strEvent.compare("eventOccurs"))
		{
			return eventOccurs(strParam);
		}
		else if (0 == strEvent.compare("IPCManageExit"))
		{
			return ipcManageExit(strParam);
		}
		else if (0 == strEvent.compare("DevManageExit"))
		{
			return devManageExit(strParam);
		}
		else if (0 == strEvent.compare("EventManageExit"))
		{
			return eventManageExit(strParam);
		}
		else if (0 == strEvent.compare("VideoManageExit"))
		{
			return videoManageExit(strParam);
		}
		else if (0 == strEvent.compare("AudioManageExit"))
		{
			return audioManageExit(strParam);
		}
		else if (0 == strEvent.compare("SpeakerManageExit"))
		{
			return speakerManageExit(strParam);
		}
		else if (0 == strEvent.compare("LogManageExit"))
		{
			return logManageExit(strParam);
		}
		else if (0 == strEvent.compare("VideoSourceExit"))
		{
			return VideoSourceExit(strParam);
		}
        else if (0 == strEvent.compare("openVideo"))
		{
			return parseRecordTime(strParam);
		}
		else if (0 == strEvent.compare("eventNoOccurs"))
		{
			return eventNoOccurs(strParam);
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	std::string CLowPowerModule::procResult(
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

	E_Module_Type CLowPowerModule::getModuleType()
	{
		return m_lowPowerInformFlow->getModuleType();
	}

	mxbool CLowPowerModule::setModuleType(E_Module_Type eType)
	{
		return m_lowPowerInformFlow->setModuleType(eType);
	}

	std::string CLowPowerModule::eventOccurs(std::string strParam)
	{
		//printf("<maix>---> eventOccurs notify\n");
		// logPrint(MX_LOG_INFOR, "eventOccurs notify");
		E_Event_Detect_Type eEventDetectType = getEventDetectType();
		if(eEventDetectType != E_EventNoOccur && 
			eEventDetectType != E_EventIdle)
		{
			setEventDetectType(E_EventOccur);
		}
		
		return procResult(std::string("200"), "","");
	}

	std::string CLowPowerModule::eventNoOccurs(std::string strParam)
	{
		// logPrint(MX_LOG_INFOR, "eventNoOccurs notify");
		E_Event_Detect_Type eEventDetectType = getEventDetectType();
		if(eEventDetectType != E_EventOccur && 
			eEventDetectType != E_EventIdle)
		{
			setEventDetectType(E_EventNoOccur);
		}

		return procResult(std::string("200"), "","");
	}

	std::string CLowPowerModule::ipcManageExit(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "ipcManageExit notify");
		
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		int iAllow = 200;

		if (jsonRoot)
		{
			cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
		
			if (jsonAlloc)
			{
				iAllow = jsonAlloc->valueint;
			}

			cJSON_Delete(jsonRoot);
		}
		
        logPrint(MX_LOG_INFOR, "ipcManageExit iAllow %d ",iAllow);
		if(iAllow == 200)
		{
			m_lowPowerInformFlow->enterLowPower(E_IPCManageModule);
		}
		else
		{
			//setModuleType(E_IPCManageModule);
		}
		
		return procResult(std::string("200"), "", "");
	}

	std::string CLowPowerModule::devManageExit(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "devManageExit notify");
		
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		
		int iAllow = 200;
		if (jsonRoot)
		{
			cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
		
			if (jsonAlloc)
			{
				iAllow = jsonAlloc->valueint;
			}

			cJSON_Delete(jsonRoot);
		}
		
		if(iAllow == 200)
		{
			m_lowPowerInformFlow->enterLowPower(E_DevManageModule);
		}
		else
		{
			//setModuleType(E_DevManageModule);
		}
		
		return procResult(std::string("200"), "", "");
	}

	std::string CLowPowerModule::eventManageExit(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "eventManageExit notify");
		
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		
		int iAllow = 200;
		if (jsonRoot)
		{
			cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
		
			if (jsonAlloc)
			{
				iAllow = jsonAlloc->valueint;
			}

			cJSON_Delete(jsonRoot);
		}
		
		if(iAllow == 200)
		{
			m_lowPowerInformFlow->enterLowPower(E_EventManageModule);
		}
		else
		{
			//setModuleType(E_EventManageModule);
		}
		return procResult(std::string("200"), "", "");
	}

	std::string CLowPowerModule::videoManageExit(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "videoManageExit notify");
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		int iAllow = 200;

		if (jsonRoot)
		{
			cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
		
			if (jsonAlloc)
			{
				iAllow = jsonAlloc->valueint;
			}

			cJSON_Delete(jsonRoot);
		}
		
		if(iAllow == 200)
		{
			m_lowPowerInformFlow->enterLowPower(E_VideoManageModule);
		}
		else
		{
			//setModuleType(E_VideoManageModule);
		}
		
		return procResult(std::string("200"), "", "");
	}

	std::string CLowPowerModule::audioManageExit(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "audioManageExit notify");
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
        int iAllow = 200;

        if (jsonRoot)
        {
            cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
        
            if (jsonAlloc)
            {
                iAllow = jsonAlloc->valueint;
            }

            cJSON_Delete(jsonRoot);
        }
        
        if(iAllow == 200)
        {
           m_lowPowerInformFlow->enterLowPower(E_AudioManageModule);
        }
		else
		{
			//setModuleType(E_AudioManageModule);
		}
		
		return procResult(std::string("200"), "", "");
	}

	std::string CLowPowerModule::speakerManageExit(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "speakerManageExit notify");
        cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
        int iAllow = 200;

        if (jsonRoot)
        {
            cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
        
            if (jsonAlloc)
            {
                iAllow = jsonAlloc->valueint;
            }

            cJSON_Delete(jsonRoot);
        }
        
        if(iAllow == 200)
        {
            logPrint(MX_LOG_INFOR, "E_SpeakerManageModule enterLowPower");
            m_lowPowerInformFlow->enterLowPower(E_SpeakerManageModule);
        }
		else
		{
			//setModuleType(E_SpeakerManageModule);
		}

		return procResult(std::string("200"), "", "");
	}

    std::string CLowPowerModule::logManageExit(std::string strParam)
    {
		logPrint(MX_LOG_INFOR, "logManageExit notify");
        cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
        int iAllow = 200;

        if (jsonRoot)
        {
            cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
        
            if (jsonAlloc)
            {
                iAllow = jsonAlloc->valueint;
            }

            cJSON_Delete(jsonRoot);
        }
        
        if(iAllow == 200)
        {
            logPrint(MX_LOG_INFOR, "E_LogManageModule enterLowPower");
            m_lowPowerInformFlow->enterLowPower(E_LogManageModule);
        }
		else
		{
			//setModuleType(E_LogManageModule);
		}

		return procResult(std::string("200"), "", "");
    }

    std::string CLowPowerModule::VideoSourceExit(std::string strParam)
    {
		logPrint(MX_LOG_INFOR, "VideoSourceExit notify");
        cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
        int iAllow = 200;

        if (jsonRoot)
        {
            cJSON *jsonAlloc = cJSON_GetObjectItem(jsonRoot, "allow");
        
            if (jsonAlloc)
            {
                iAllow = jsonAlloc->valueint;
            }

            cJSON_Delete(jsonRoot);
        }
        
        if(iAllow == 200)
        {
            logPrint(MX_LOG_INFOR, "E_VideoSourceModule enterLowPower");
            m_lowPowerInformFlow->enterLowPower(E_VideoSourceModule);
        }
		else
		{
			//setModuleType(E_VideoSourceModule);
		}

		return procResult(std::string("200"), "", "");
    }

    std::string CLowPowerModule::parseRecordTime(std::string strParam)
    {
        cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
        int iRecordTime = 0;
        if (jsonRoot)
        {
            cJSON *jsonRecord = cJSON_GetObjectItem(jsonRoot, "recordTime");
            if (!jsonRecord)
            {
                cJSON_Delete(jsonRoot);
                return procResult(std::string("500"),"",
                    std::string("parse recordtime failed"));
            }
            else
            {
                iRecordTime = jsonRecord->valueint;
            }
            cJSON_Delete(jsonRoot);
        }
        if(iRecordTime >= 0)
        {
            m_iUserSetRecordTime = iRecordTime;
            logPrint(MX_LOG_INFOR,"m_iUserSetRecordTime = %d",m_iUserSetRecordTime);
        }
        return procResult(std::string("200"),"",
                    std::string("get recordtime success"));
    }

	void CLowPowerModule::eventDetectProc()
	{
        prctl(PR_SET_NAME,"lowpower_detect");
        printf("CLowPowerModule eventDetectProc\n");
        logPrint(MX_LOG_INFOR, "CLowPowerModule eventDetectProc");
		while (1)
		{
			switch (m_eEventDetectType)
			{
			case E_EventDetectStart:
			{
				if (m_iCumulativeTime > m_iEventDetectTime)
				{
					setEventDetectType(E_EventNoOccur);
				}
			}
			break;
			case E_EventNoOccur:
			{
				E_Event_Detect_Type eLastEventDetectType =
					getLastEventDetectType();

				if (eLastEventDetectType == E_EventDetectStart)
				{
					printf("<maix>---> start no event occur: %d\n", m_iEventDetectTime);
					logPrint(MX_LOG_INFOR, "start no event occur %d ",m_iEventDetectTime);
					setModuleType(E_VideoSourceModule);
				}
				else if (eLastEventDetectType == E_EventOccur)
				{
					printf("<maix>---> record no event occur\n");
					logPrint(MX_LOG_INFOR, "record no event occur");
					setModuleType(E_VideoSourceModule);
				}
				
				setEventDetectType(E_EventIdle);
			}
			break;
			case E_EventOccur:
			{
				if (m_iCumulativeTime > 3)
				{
					//printf("<maix>--->low power: m_iCumulativeTime > 3\n");
                    logPrint(MX_LOG_INFOR,"E_EventOccur no event 3");
					setEventDetectType(E_EventNoOccur);
				}

				m_iCumulativeRecordTime++;
				//printf("<maix>--->low power: m_iCumulativeRecordTime <%d>\n", 
				//	m_iCumulativeRecordTime);
				if ((m_iCumulativeRecordTime > m_iRecordLastTime) || (m_iCumulativeRecordTime > (m_iUserSetRecordTime + 5 )))
				{
                    logPrint(MX_LOG_INFOR,"m_iCumulativeRecordTime = %d m_iRecordLastTime = %d m_iUserSetRecordTime = %d",m_iCumulativeRecordTime,m_iRecordLastTime,m_iUserSetRecordTime);
					setEventDetectType(E_EventNoOccur);
				}
			}
			break;
			case E_EventIdle:
			{
				//printf("<maix>--->low power E_EventIdle\n");
			}
			break;
			default:
				break;
			}

#ifdef _WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
			m_iCumulativeLongLastTime++;
			if (m_iCumulativeLongLastTime > m_iLongLastTime)
			{
				// printf("<maix>--->low power: m_iCumulativeLongLastTime<%d> m_iLongLastTime<%d>\n", 
				// 	m_iCumulativeLongLastTime,
				// 	m_iLongLastTime);
                logPrint(MX_LOG_INFOR,"time out to enter lowpower");
                setModuleType(E_VideoSourceModule);
                usleep(5 * 1000 * 1000);
				
			}
			m_iCumulativeTime++;
		}
	}

	E_Event_Detect_Type CLowPowerModule::getLastEventDetectType()
	{
		std::unique_lock<std::mutex> lock(m_mutexEventDetectType);
		return m_eLastEventDetectType;
	}

	E_Event_Detect_Type CLowPowerModule::getEventDetectType()
	{
		std::unique_lock<std::mutex> lock(m_mutexEventDetectType);
		return m_eEventDetectType;
	}
	
	mxbool CLowPowerModule::setEventDetectType(E_Event_Detect_Type eType)
	{
		std::unique_lock<std::mutex> lock(m_mutexEventDetectType);
		m_eLastEventDetectType = m_eEventDetectType;
		m_eEventDetectType = eType;
		m_iCumulativeTime = 0;
	
		
		return mxtrue;
	}
}
