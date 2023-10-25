#include "speaker_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "speaker_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#include "log_mx.h"

namespace maix {
	CSpeakerManageModule::CSpeakerManageModule(std::string strGUID,
		std::string strName)
		: CModule(strGUID, strName)
	{
	
	}

	CSpeakerManageModule::~CSpeakerManageModule()
	{
	}

	mxbool CSpeakerManageModule::init()
	{
		if (!initConnectModule())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		if (!initLowPower())
			return mxfalse;
        
        if (!initSpeakerSourceRemote())
            return mxfalse;

		return mxtrue;
	}

	mxbool CSpeakerManageModule::unInit()
	{
		return mxbool();
	}

	mxbool CSpeakerManageModule::initServer()
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
					int iType = 4;
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
						CSpeakerManageRemoteEventServer * objSpeakerManageRemoteEventServer =
							new CSpeakerManageRemoteEventServer(this);

						if (!objSpeakerManageRemoteEventServer)
							return mxfalse;

						objSpeakerManageRemoteEventServer->init();

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
							objSpeakerManageRemoteEventServer);

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

					if (strName.compare("speaker_channel_1_server") == 0)
					{
						CSpeakerSourceInputServer * objSpeakerSourceInputServer =
							new CSpeakerSourceInputServer(this);

						if (!objSpeakerSourceInputServer)
							return mxfalse;
						
						if (!objSpeakerSourceInputServer->init(strName, iType,
							strIP,iPort,strUnix,iLen))
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

						objServerEndPoint->init(serverConfig, objSpeakerSourceInputServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;

						if (!addSpeakerChannel(strName, objSpeakerSourceInputServer))
						{
							return mxfalse;
						}
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CSpeakerManageModule::initConnectModule()
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

	mxbool CSpeakerManageModule::initLowPower()
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

    mxbool CSpeakerManageModule::initSpeakerSourceRemote()
    {
        if (!getConfig("SPEAKER_SOURCE_REMOTE_EVENT", 
            "GUID", m_strSpeakerSourceGUID))
        {
            return mxfalse;
        }

        if (!getConfig("SPEAKER_SOURCE_REMOTE_EVENT",
            "SERVER", m_strSpeakerSourceServer))
        {
            return mxfalse;
        }
        return mxtrue;
    }

	void CSpeakerManageModule::lowPowerRun()
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

	std::string CSpeakerManageModule::lowPowerProc()
	{

        logPrint(MX_LOG_ERROR,"speaker enter lowpower");

        mxbool bSpeakerUninit = sendSpeakerSourceUninit();

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "SpeakerManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", bSpeakerUninit ? 200 : 500);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

    mxbool CSpeakerManageModule::sendSpeakerSourceUninit()
    {
        // cJSON *jsonRoot = cJSON_CreateObject();
        // cJSON *jsonParam = cJSON_CreateObject();

        // cJSON_AddStringToObject(jsonRoot, "event", "SpeakerSourceUninit");
		// cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        // char *out = cJSON_Print(jsonRoot);
        // std::string strSpeakerSource = std::string(out);
        // cJSON_Delete(jsonRoot);
        // if (out)
        //     free(out);  
        
        // std::string strResult = output(m_strSpeakerSourceGUID,
        //     m_strSpeakerSourceServer,
        //     (unsigned char*)strSpeakerSource.c_str(), strSpeakerSource.length());

        // logPrint(MX_LOG_ERROR,"sendSpeakerSourceUninit result %s",strResult.c_str());
        // if (strResult.length() > 0)
        // {
        //     std::string strCode;
        //     std::string strErrMsg;
        //     cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

        //     if (jsonRoot)
        //     {
        //         cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
        //         if (jsonCode)
        //         {
        //             strCode = std::string(jsonCode->valuestring);
        //         }
        //         cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
        //         if (jsonErrMsg)
        //         {
        //             char *pcErrMsg = cJSON_Print(jsonErrMsg);
        //             if (pcErrMsg)
        //             {
        //                 strErrMsg = std::string(pcErrMsg);
        //                 free(pcErrMsg);
        //             }

        //         }
        //         cJSON_Delete(jsonRoot);
        //     }

        //     if (strCode.compare("200") == 0)
        //     {
        //         return mxtrue;
        //     }
        //     else
        //     {
        //         return mxfalse;
        //     }

        // }
        // else
        // {
        //     return mxtrue;
        // }
        return mxtrue;
    }

	std::string CSpeakerManageModule::remoteEventServerProc(std::string strEvent, 
		std::string strParam)
	{
		if (0 == strEvent.compare("openSpeaker"))
		{
			return open(strParam);
		}
		else if (0 == strEvent.compare("closeSpeaker"))
		{
			return close(strParam);
		}
		else if (0 == strEvent.compare("configSpeaker"))
		{
			return config(strParam);
		}
		else if (0 == strEvent.compare("resetSpeaker"))
		{
			return reset(strParam);
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

	mxbool CSpeakerManageModule::addSpeakerChannel(std::string strName,
		CSpeakerSourceInputServer * objSpeakerSourceInputServer)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexSpeakerInputServer);
			m_mapSpeakerInputServer[strName] = objSpeakerSourceInputServer;
		}
		std::unique_lock<std::mutex> lock(m_mutexSpeakerChn);
		std::shared_ptr<CSpeakerManageChannel> channel(
			new CSpeakerManageChannel(this, strName, objSpeakerSourceInputServer));
		channel->init();
		m_mapSpeakerChn[strName] = channel;

		while (!channel->open())
		{
#ifdef _WIN32
			Sleep(200);
#else
			usleep(200*1000);
#endif
		}

		m_mapSpeakerChnProc[strName] = std::thread([channel]() {
			channel->run();
		});

		return mxtrue;
	}

	std::string CSpeakerManageModule::open(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strModuleName;
		std::string strChannelName;

		if (jsonRoot)
		{
			cJSON *jsonModuleGUID = cJSON_GetObjectItem(jsonRoot, "moduleGUID");
			if (!jsonModuleGUID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
					std::string("moduleName param parse failed"));
			}
			else
			{
				strModuleName = std::string(jsonModuleName->valuestring);
			}

			cJSON *jsonChannelID = cJSON_GetObjectItem(jsonRoot, "channelID");
			if (!jsonChannelID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("channelID param parse failed"));
			}
			else
			{
				strChannelName = std::string(jsonChannelID->valuestring);
			}

			cJSON_Delete(jsonRoot);
		}

		CSpeakerSourceInputServer* objSpeakerSourceChannel =
			m_mapSpeakerInputServer[strChannelName];

		if (objSpeakerSourceChannel)
		{
			return objSpeakerSourceChannel->open(strParam);
		}
		else
		{
			return procResult(std::string("400"), "",
				strChannelName.append(" not found"));
		}

		return procResult(std::string("200"), "",
			strChannelName.append("open success"));
	}

	std::string CSpeakerManageModule::close(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerManageModule::config(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerManageModule::reset(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerManageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		return procResult(std::string("200"), "",
			"speaker manage enter low power");
	}

	std::string CSpeakerManageModule::procResult(std::string code, 
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