#include "audio_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "audio_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"

namespace maix {
	CAudioManageModule::CAudioManageModule(std::string strGUID, 
		std::string strName)
		: CModule(strGUID, strName)
	{
	}

	CAudioManageModule::~CAudioManageModule()
	{
	}

	mxbool CAudioManageModule::init()
	{
		if (!initConnectModule())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		if (!initLowPower())
			return mxfalse;

		return mxtrue;
	}

	mxbool CAudioManageModule::unInit()
	{
	
		return mxbool();
	}

	mxbool CAudioManageModule::initServer()
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
						CAudioManageRemoteEventServer * objAudioManageRemoteEventServer =
							new CAudioManageRemoteEventServer(this);

						if (!objAudioManageRemoteEventServer)
							return mxfalse;

						objAudioManageRemoteEventServer->init();

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
							objAudioManageRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
				else if (strComType.compare("RCF_AUDIO") == 0)
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

					if (strName.compare("audio_channel_1_server") == 0)
					{
						CAudioSourceInputServer * objAudioSourceInputServer =
							new CAudioSourceInputServer(this);

						if (!objAudioSourceInputServer)
							return mxfalse;

						objAudioSourceInputServer->init(strName, iType,
							strIP,
							iPort,
							strUnix,
							iLen);

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

						objServerEndPoint->init(serverConfig, objAudioSourceInputServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;

						if (!addAudioChannel(strName, objAudioSourceInputServer))
						{
							return mxfalse;
						}
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CAudioManageModule::initConnectModule()
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
						int iType = 4;
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

	mxbool CAudioManageModule::initLowPower()
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

	void CAudioManageModule::lowPowerRun()
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

	std::string CAudioManageModule::lowPowerProc()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "AudioManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

	std::string CAudioManageModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		if (0 == strEvent.compare("openAudio"))
		{
			return openAudio(strParam);
		}
		else if (0 == strEvent.compare("closeAudio"))
		{
			return closeAudio(strParam);
		}
		else if (0 == strEvent.compare("configAudio"))
		{
			return configAudio(strParam);
		}
		else if (0 == strEvent.compare("resetAudio"))
		{
			return resetAudio(strParam);
		}
		else if (0 == strEvent.compare("EnterLowPower"))
		{
			return enterLowPower(strParam);
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}

	}

	mxbool CAudioManageModule::addAudioChannel(std::string strName,
		CAudioSourceInputServer * objAudioSourceInputServer)
	{
		std::unique_lock<std::mutex> lock(m_mutexAudioChn);
		std::shared_ptr<CAudioManageChannel> channel(
			new CAudioManageChannel(this, strName, objAudioSourceInputServer));
		if (!channel->init())
		{
			return mxfalse;
		}
		m_mapAudioChn[strName] = channel;
		m_mapAudioChnProc[strName] = std::thread([channel]() {
			channel->run();
		});

		std::shared_ptr<CAudioManageChannelSession> channelSession(
			new CAudioManageChannelSession(objAudioSourceInputServer));

		if (!channelSession->init())
		{
			return mxfalse;
		}

		m_mapAudioChnSession[strName] = channelSession;
		m_mapAudioChnSessionProc[strName] = std::thread([channelSession]() {
			channelSession->run();
		});
		return mxtrue;
	}

	std::string CAudioManageModule::openAudio(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strModuleName;
		std::string strKey=std::string("");
		std::string strChannelName;
		std::string strServerName;
		int iType = 0;
		std::string strIP;
		int iPort = 0;
		std::string strUnix;
		int iLen = 0;
		if (jsonRoot)
		{
			cJSON *jsonModuleGUID = cJSON_GetObjectItem(jsonRoot, "moduleGUID");
			if (!jsonModuleGUID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("moduleGUID param parse failed"));
			}
			else
			{
				strModuleGUID = std::string(jsonModuleGUID->valuestring);
			}

			cJSON *jsonModuleName = cJSON_GetObjectItem(jsonRoot, "moduleName");
			if (!jsonModuleName)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("moduleName param parse failed"));
			}
			else
			{
				strModuleName = std::string(jsonModuleName->valuestring);
			}

			cJSON *jsonKey = cJSON_GetObjectItem(jsonRoot, "key");
			if (!jsonKey)
			{
				//cJSON_Delete(jsonRoot);
				//return procResult(std::string("500"), "",
					//std::string("key param parse failed"));
			}
			else
			{
				strKey = std::string(jsonKey->valuestring);
			}

			cJSON *jsonServerName = cJSON_GetObjectItem(jsonRoot, "serverName");
			if (!jsonServerName)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("serverName param parse failed"));
			}
			else
			{
				strServerName = std::string(jsonServerName->valuestring);
			}

			cJSON *jsonChannelID = cJSON_GetObjectItem(jsonRoot, "channelID");
			if (!jsonChannelID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("channelID param parse failed"));
			}
			else
			{
				strChannelName = std::string(jsonChannelID->valuestring);
			}

			cJSON *jsonType = cJSON_GetObjectItem(jsonRoot, "type");
			if (!jsonType)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("type param parse failed"));
			}
			else
			{
				iType = jsonType->valueint;
			}

			cJSON *jsonIP = cJSON_GetObjectItem(jsonRoot, "IP");
			if (!jsonIP)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("IP param parse failed"));
			}
			else
			{
				strIP = std::string(jsonIP->valuestring);
			}

			cJSON *jsonPort = cJSON_GetObjectItem(jsonRoot, "port");
			if (!jsonPort)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("port param parse failed"));
			}
			else
			{
				iPort = jsonPort->valueint;
			}

			cJSON *jsonUnix = cJSON_GetObjectItem(jsonRoot, "unix");
			if (!jsonUnix)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("unix param parse failed"));
			}
			else
			{
				strUnix = std::string(jsonUnix->valuestring);
			}

			cJSON *jsonLen = cJSON_GetObjectItem(jsonRoot, "len");
			if (!jsonLen)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("len param parse failed"));
			}
			else
			{
				iLen = jsonLen->valueint;
			}

			cJSON_Delete(jsonRoot);
		}

		std::shared_ptr<CIModule> imodule = NULL;
		if (!getConnectModule(strModuleGUID, imodule))
		{
			std::shared_ptr<CModule> newModule(
				new CModule(strModuleGUID, strModuleName));

			imodule = newModule;

			if (!connect(imodule))
			{
				return procResult(std::string("500"),"",
					strChannelName.append("create module failed"));
			}

		}
		CModule *module = dynamic_cast<CModule *>(imodule.get());
		std::shared_ptr<CAudioManageChannel> objAudioManageChannel =
			m_mapAudioChn[strChannelName];

		if (objAudioManageChannel)
		{
			std::shared_ptr<CComRCFClientEndPoint> objClientEndPoint(
				new CComRCFClientEndPoint(this));

			T_COM_PROXY_CLIENT_CONFIG clientConfig;
			// memset(&clientConfig, 0, sizeof(T_COM_PROXY_CLIENT_CONFIG));

			clientConfig.m_iType = iType;
			clientConfig.m_strIP = strIP;
			clientConfig.m_iPort = iPort;
			clientConfig.m_strUnix = strUnix;
			clientConfig.m_iTCPMsgLen = iLen;
			clientConfig.m_iUDPMsgLen = iLen;
			clientConfig.m_iUNIXMsgLen = iLen;


			if (!module->regClient(strServerName, objClientEndPoint))
			{
				return procResult(std::string("500"),"",
					std::string("client reg failed"));
			}
			else
			{
				objClientEndPoint->init(clientConfig, E_CLIENT_FRAME);
			}
				
			if (!objAudioManageChannel->open(
				strModuleGUID, strServerName, strKey))
			{
				return procResult(std::string("500"),"",
					strChannelName.append("open failed"));
			}
		}
		else
		{
			return procResult(std::string("400"),"",
				strChannelName.append(" not found"));
		}
			
		return procResult(std::string("200"),"",
			strChannelName.append("open success"));
	}

	std::string CAudioManageModule::closeAudio(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strChannelName;
		std::string strServerName;
		
		if (jsonRoot)
		{
			cJSON *jsonModuleGUID = cJSON_GetObjectItem(jsonRoot, "moduleGUID");
			if (!jsonModuleGUID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("moduleID param parse failed"));
			}
			else
			{
				strModuleGUID = std::string(jsonModuleGUID->valuestring);
			}

			cJSON *jsonServerName = cJSON_GetObjectItem(jsonRoot, "serverName");
			if (!jsonServerName)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("serverName param parse failed"));
			}
			else
			{
				strServerName = std::string(jsonServerName->valuestring);
			}

			cJSON *jsonChannelID = cJSON_GetObjectItem(jsonRoot, "channelID");
			if (!jsonChannelID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("channelID param parse failed"));
			}
			else
			{
				strChannelName = std::string(jsonChannelID->valuestring);
			}

			cJSON_Delete(jsonRoot);
		}

		std::shared_ptr<CIModule> imodule = NULL;
		if (!getConnectModule(strModuleGUID, imodule))
		{
            logPrint(MX_LOG_ERROR,"audio get module failed");
		}
		else
		{
			imodule->unRegClient(strServerName);
		}

		if (m_mapAudioChn.count(strChannelName) != 0)
		{
			std::shared_ptr<CAudioManageChannel> objAudioManageChannel =
				m_mapAudioChn[strChannelName];
			if (objAudioManageChannel)
			{
				objAudioManageChannel->close(strModuleGUID, strServerName);
			}
			else
			{
				// return  procResult(std::string("500"),"",
				// 	strChannelName.append(" not exist"));
                logPrint(MX_LOG_ERROR,"not exist");
			}

		}
		else
		{
			return procResult(std::string("500"),"",
				strChannelName.append(" not exist"));
		}
		
		return procResult(std::string("200"),"",
			strChannelName.append(" close success"));
	}

	std::string CAudioManageModule::configAudio(std::string strParam)
	{
		return std::string();
	}

	std::string CAudioManageModule::resetAudio(std::string strParam)
	{
		return std::string();
	}

	std::string CAudioManageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		return procResult(std::string("200"), "",
			"audio manage enter low power");
	}

	std::string CAudioManageModule::procResult(std::string code,
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
}
