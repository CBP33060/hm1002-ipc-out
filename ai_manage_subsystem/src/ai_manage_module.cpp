#include "ai_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "ai_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#include "fw_env_para.h"

namespace maix {
	CAIManageModule::CAIManageModule(std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
	{
	}

	CAIManageModule::~CAIManageModule()
	{
	}

	mxbool CAIManageModule::init()
	{
		if (!addEventManageRemoteEvent())
			return mxfalse;

		if(!initDevEventServer())
			return mxfalse;

		if (!initConnectModule())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		char *pValue = getFWParaConfig(PARA_AI_DETECTION_MASK);
		if(pValue != NULL)
			m_iDetectMask = atoi(pValue);
		else
			m_iDetectMask = 0;

		pValue = getFWParaConfig(PARA_AREA_DETECT_COORD);
		if(pValue != NULL && strcmp(pValue,"0"))
			syncDetectArea(std::string(pValue));
		else
			m_vecAreaDetecMask.clear();

		int iStayTimeArry[3] = {0,3,6};
		pValue = getFWParaConfig(PARA_PERSON_STAY_TIME);
		if(pValue != NULL)
			m_iStaytime = iStayTimeArry[atoi(pValue)];
		else
			m_iStaytime = iStayTimeArry[0];

		return mxtrue;
	}

	mxbool CAIManageModule::unInit()
	{
		std::map<std::string, std::shared_ptr<CVideoManageChannel>> ::iterator iter;
		for (iter = m_mapVideoChn.begin(); iter != m_mapVideoChn.end(); iter++)
		{
			iter->second->unInit();
		}

		return mxtrue;
	}

	mxbool CAIManageModule::initServer()
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
						CAIManageRemoteEventServer * objAIManageRemoteEventServer =
							new CAIManageRemoteEventServer(this);

						if (!objAIManageRemoteEventServer)
							return mxfalse;

						objAIManageRemoteEventServer->init();

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
							objAIManageRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
				else if (strComType.compare("RCF_VIDEO") == 0)
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

					if (strName.compare("video_channel_5_server") == 0)
					{
						CVideoSourceInputServer * objVideoSourceInputServer =
							new CVideoSourceInputServer(this);

						if (!objVideoSourceInputServer)
							return mxfalse;

						if (!objVideoSourceInputServer->init(strName, iType,
							strIP,
							iPort,
							strUnix,
							iLen))

							return mxfalse;

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

						objServerEndPoint->init(serverConfig, objVideoSourceInputServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;

						if (!addVideoChannel(strName, objVideoSourceInputServer))
						{
							return mxfalse;
						}
						
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CAIManageModule::initConnectModule()
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
						int iType = 0;
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
					else if (strComType.compare("RCF_VIDEO") == 0)
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

						objClientEndPoint->init(clientConfig, E_CLIENT_FRAME);

						if (!module->regClient(strName, objClientEndPoint))
							return mxfalse;

					}

				}
			}

		}

		return mxtrue;
	}

	mxbool CAIManageModule::addVideoChannel(std::string strName,
		CVideoSourceInputServer * objVideoSourceInputServer)
	{
		std::unique_lock<std::mutex> lock(m_mutexVideoChn);
		std::shared_ptr<CVideoManageChannel> channel(
			new CVideoManageChannel(this, strName, objVideoSourceInputServer));
		if (!channel->init(m_eventManageRemoteEvent))
			return mxfalse;

		m_mapVideoChn[strName] = channel;
		m_mapVideoChnProc[strName] = std::thread([channel]() {
			channel->run();
		});

		/*std::shared_ptr<CVideoManageChannelSession> channelSession(
			new CVideoManageChannelSession(objVideoSourceInputServer));

		if (!channelSession->init())
		{
			return mxfalse;
		}

		m_mapVideoChnSession[strName] = channelSession;
		m_mapVideoChnSessionProc[strName] = std::thread([channelSession]() {
			channelSession->run();
		});*/

		return mxtrue;
	}

	mxbool CAIManageModule::addEventManageRemoteEvent()
	{
		std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent(
			new CEventManageRemoteEvent(this));

		std::string strEventManageGUID;
		if (!getConfig("EVENT_MANAGE_REMOTE_EVENT", "GUID", strEventManageGUID))
		{
			return mxfalse;
		}

		std::string strEventManageRemoteEventServer;
		if (!getConfig("EVENT_MANAGE_REMOTE_EVENT", "SERVER", 
			strEventManageRemoteEventServer))
		{
			return mxfalse;
		}

		if (!eventManageRemoteEvent->init(strEventManageGUID, 
			strEventManageRemoteEventServer))
		{
			return mxfalse;
		}
		m_eventManageRemoteEvent = eventManageRemoteEvent;
		m_eventManageRemoteEventThread = std::thread([eventManageRemoteEvent]() {
			eventManageRemoteEvent->run();
		});

		return mxtrue;
	}

	std::string CAIManageModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "ai remoteEventServerProc strEvent=[%s], strParam=[%s]",
			strEvent.c_str(),strParam.c_str());
		
		if(strEvent == "devConfig")
		{
			std::string strConfigName;
			std::string strValue;

			cJSON* jsonRoot = cJSON_Parse(strParam.c_str());

			if(jsonRoot)
			{
				cJSON *jsonConfigName = cJSON_GetObjectItem(jsonRoot, "configName");
				if(jsonConfigName)
				{
					strConfigName = std::string(jsonConfigName->valuestring);
				}
				cJSON *jsonConfigValue = cJSON_GetObjectItem(jsonRoot, "configParam");
				if(jsonConfigValue)
				{
					cJSON *jsonValue = cJSON_GetObjectItem(jsonConfigValue, "value");
					if(jsonValue)
					{
						strValue = std::string(jsonValue->valuestring);
					}
				}

				cJSON_Delete(jsonRoot);
			}

			if (0 == strConfigName.compare(EVENT_SET_PEOPLE_DETECTION))
			{
				syncDetectSwitch(E_AI_PERSON_APPEAR, strValue);
				syncDetectSwitch(E_AI_PERSON_STAY, strValue);
			}
			else if(0 == strConfigName.compare(EVENT_SET_ANIMAL_DETECTION))
			{
				syncDetectSwitch(E_AI_ANIMAL_PET_APPEAR, strValue);
				//syncDetectSwitch(E_AI_ANIMAL_DOG_DETECTION, strValue);
			}
			else if(0 == strConfigName.compare(EVENT_SET_PACKAGE_DETECTION))
			{
				syncDetectSwitch(E_AI_PACKAGE_ENTER, strValue);
				syncDetectSwitch(E_AI_PACKAGE_MOVE, strValue);
				syncDetectSwitch(E_AI_PACKAGE_LEAVE, strValue);
			}
			else if(0 == strConfigName.compare(EVENT_SET_VEHICLE_DETECTION))
			{
				syncDetectSwitch(E_AI_CAR_APPEAR, strValue);
				syncDetectSwitch(E_AI_CAR_STAY, strValue);
			}
			else if(0 == strConfigName.compare(EVENT_SET_PEOPLE_STAY_TIME))
			{
				syncDetectStayTime(strValue);
			}
			else if(0 == strConfigName.compare(EVENT_SET_AREA_DETECT_COORD))
			{
				syncDetectArea(strValue);
			}
			else
			{
				return procResult(std::string("400"), 
					strEvent.append("  event not support"));
			}
		}
		else if (strEvent == "closeAIDetect")
		{
			return closeAIDetect();
		}
		else if (strEvent == "openAIDetect")
		{
			return openAIDetect();
		}
		else
		{
			return procResult(std::string("400"),  
				strEvent.append("event not supported"));
		}

		return procResult(std::string("200"),  
			strEvent.append(" ok"));
		
	}

	std::string CAIManageModule::closeAIDetect()
	{
		std::unique_lock<std::mutex> lock(m_mutexVideoChn);

		m_eventManageRemoteEvent->closeAIEventPush();
		for (auto iter = m_mapVideoChn.begin(); iter != m_mapVideoChn.end(); ++iter)
		{
			iter->second->closeDetect();
		}

		for (auto iter = m_mapVideoChnSession.begin(); iter != m_mapVideoChnSession.end(); ++iter)
		{
			iter->second->setState(E_CLOSEING);
		}
		
		return procResult(std::string("200"),  
			"close AI Deteck ok");
	}

	std::string CAIManageModule::openAIDetect()
	{
		std::unique_lock<std::mutex> lock(m_mutexVideoChn);

		m_eventManageRemoteEvent->openAIEventPush();
		for (auto iter = m_mapVideoChn.begin(); iter != m_mapVideoChn.end(); ++iter)
		{
			iter->second->openDetect();
		}
		for (auto iter = m_mapVideoChnSession.begin(); iter != m_mapVideoChnSession.end(); ++iter)
		{
			iter->second->setState(E_OPENING);
		}

		return procResult(std::string("200"),  
			"open AI Deteck ok");
	}

	std::string CAIManageModule::procResult(
		std::string code, std::string strMsg)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
		cJSON_AddStringToObject(jsonRoot, "errMsg", strMsg.c_str());
		char *pcResult = cJSON_Print(jsonRoot);
		strResult = std::string(pcResult);
		cJSON_Delete(jsonRoot);
		if (pcResult)
			free(pcResult);

		return strResult;
	}

	int CAIManageModule::getDetectMask()
	{
		std::unique_lock<std::mutex> lock(m_mutexDete);
		return m_iDetectMask;
	}

	int CAIManageModule::getStaytime()
	{
		std::unique_lock<std::mutex> lock(m_mutexStayTime);
		return m_iStaytime;
	}

	std::vector<std::vector<float>> CAIManageModule::getAreaDetecMask()
	{
		std::unique_lock<std::mutex> lock(m_mutexArea);
		return m_vecAreaDetecMask;
	}

	void CAIManageModule::syncDetectSwitch(E_AI_DETECTION_TYPE eValue, std::string strParam)
	{
		std::unique_lock<std::mutex> lock(m_mutexDete);

		int iValue = atoi(strParam.c_str());
		if(iValue == true)
			m_iDetectMask |= (1 << eValue);
		else
			m_iDetectMask &= (~(1 << eValue));

		std::string strValue = std::to_string(m_iDetectMask);
		setFWParaConfig(PARA_AI_DETECTION_MASK,strValue.c_str(),1);
		saveFWParaConfig();
		logPrint(MX_LOG_DEBUG, "change detect mask, set value:%d, mask:%d", eValue, m_iDetectMask);
	}

	void CAIManageModule::syncDetectArea(std::string strParam)
	{	
		std::unique_lock<std::mutex> lock(m_mutexArea);

		std::vector<float> vecAreaDetecMask;
		m_vecAreaDetecMask.erase(m_vecAreaDetecMask.begin(),m_vecAreaDetecMask.end());

		cJSON* jsonVal = cJSON_Parse(strParam.c_str());
		if(jsonVal && cJSON_IsArray(jsonVal))
		{
			int iSize = cJSON_GetArraySize(jsonVal);
			for(int i = 0; i < iSize; i++)
			{
				cJSON* jsonValIndex = cJSON_GetArrayItem(jsonVal, i);
				cJSON* jsonIfUsing = cJSON_GetObjectItem(jsonValIndex, "if_using");
				bool isFlag = cJSON_IsTrue(jsonIfUsing);
				if(isFlag)
				{
					vecAreaDetecMask.clear();

					cJSON* jsonLeftTop = cJSON_GetObjectItem(jsonValIndex, "leftTop");
					cJSON* jsonLeftBottom = cJSON_GetObjectItem(jsonValIndex, "leftBottom");
					cJSON* jsonRightTop = cJSON_GetObjectItem(jsonValIndex, "rightTop");
					cJSON* jsonRightBottom = cJSON_GetObjectItem(jsonValIndex, "rightBottom");

					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonLeftTop, "x")->valuedouble)) / YUV_FRAME_WIDTH);
					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonLeftTop, "y")->valuedouble)) / YUV_FRAME_HEIGHT);

					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonLeftBottom, "x")->valuedouble)) / YUV_FRAME_WIDTH);
					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonLeftBottom, "y")->valuedouble)) / YUV_FRAME_WIDTH);

					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonRightTop, "x")->valuedouble)) / YUV_FRAME_WIDTH);
					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonRightTop, "y")->valuedouble)) / YUV_FRAME_WIDTH);

					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonRightBottom, "x")->valuedouble)) / YUV_FRAME_WIDTH);
					vecAreaDetecMask.push_back(((float)(cJSON_GetObjectItem(jsonRightBottom, "y")->valuedouble)) / YUV_FRAME_WIDTH);
					
					m_vecAreaDetecMask.push_back(vecAreaDetecMask);

					for (const auto& area : vecAreaDetecMask)
					{
						logPrint(MX_LOG_DEBUG, "sync detect area: %f", area);
					}

				}
			}

			setFWParaConfig(PARA_AREA_DETECT_COORD,strParam.c_str(),1);
			saveFWParaConfig();
		}
	}

	void CAIManageModule::syncDetectStayTime(std::string strParam)
	{
		std::unique_lock<std::mutex> lock(m_mutexStayTime);
		m_iStaytime = atoi(strParam.c_str());

		std::map<std::string, std::shared_ptr<CVideoManageChannel>> ::iterator iter;
		for (iter = m_mapVideoChn.begin(); iter != m_mapVideoChn.end(); iter++)
		{
			iter->second->syncDetectStayTime(m_iStaytime);
		}

		logPrint(MX_LOG_DEBUG, "sync detect stay time, set mask:%d", m_iStaytime);
	}

	mxbool CAIManageModule::initDevEventServer()
	{
		if(!getConfig("DEV_MANAGE_REMOTE_EVENT", "GUID", m_strDevManageGUID))
		{
			return mxfalse;
		}
		if(!getConfig("DEV_MANAGE_REMOTE_EVENT", "SERVER", m_strDevManageServer))
		{
			return mxfalse;
		}
		return mxtrue;
	}

	void CAIManageModule::sendToDevAiEvent()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "AiEvent");
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);
         
        output(m_strDevManageGUID,m_strDevManageServer,(unsigned char*) strResult.c_str(),strResult.length());
	}

}
