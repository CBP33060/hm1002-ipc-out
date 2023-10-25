#include "speaker_source_module.h"
#include "media_interface_factory.h"
#include <list>
#include <thread>
#include "com_client_end_point.h"
#include "com_server_end_point.h"
#include "com_rpc_client_end_point.h"
#include "com_rpc_server_end_point.h"
#include "cJSON.h"
#include "speaker_source_remote_event_server.h"
#include "log_mx.h"

namespace maix {
	CSpeakerSourceModule::CSpeakerSourceModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
	{
	}

	CSpeakerSourceModule::~CSpeakerSourceModule()
	{
	}

	mxbool CSpeakerSourceModule::init()
	{
		if (!initServer())
			return mxfalse;

		std::string strInterfaceConfigPath;
		if (!getConfig("INTERFACE_CONFIG", "PATH", strInterfaceConfigPath))
		{
			return mxfalse;
		}

		std::list<std::string> listInterfaces;
		listInterfaces = CMediaInterfaceFactory::interfaceList();
		std::string strInterface;
		std::list<std::string>::iterator itl;
		for (itl = listInterfaces.begin(); 
				itl != listInterfaces.end(); itl++)
		{
			strInterface = *itl;
			if (strInterface.empty())
			{
				continue;
			}

			MediaInterfaceInfo info =
				CMediaInterfaceFactory::getInfo(strInterface);
			if (info.m_strType.compare("speaker") == 0)
			{
				LP_MEDIA_INTERFACE pInterface =
					CMediaInterfaceFactory::create(strInterface, strInterface);

				std::shared_ptr<CMediaInterface> objInterface =
					pInterface->getInterface(strInterface);

#ifdef _WIN32
				std::string strInterfaceConfig = 
					strInterfaceConfigPath + strInterface + ".ini";
#else
				std::string strInterfaceConfig = 
					"/etc/70mai/media_interface/" + strInterface + ".ini";
#endif

				if (objInterface->loadConfig(strInterfaceConfig))
				{
					if (objInterface->init())
					{
						if (!addInterface(strInterface, objInterface))
						{
							logPrint(MX_LOG_ERROR, "add interface failed: %s", 
								strInterface.c_str());
							return mxfalse;
						}
					}
					else
					{
						logPrint(MX_LOG_ERROR, "init interface failed: %s", 
								strInterface.c_str());
						return mxfalse;
					}

				}
				else
				{
					logPrint(MX_LOG_ERROR, "loadconfig interface failed: %s", 
								strInterface.c_str());
					return mxfalse;
				}
			}

		}

		return mxtrue;
	}

	mxbool CSpeakerSourceModule::unInit()
	{
		return mxtrue;
	}

	mxbool CSpeakerSourceModule::addInterface(std::string strName, 
		std::shared_ptr<CMediaInterface> mediaInterface)
	{
		int iChnNum = mediaInterface->getChnNum();

		if (iChnNum <= 0)
		{
			return mxfalse;
		}

		for (int i = 0; i < iChnNum; i++)
		{
			std::string strChannelName = mediaInterface->getChnName(i);
			int iChannelSN = mediaInterface->getChnSN(i);
			E_P_TYPE ePacketType = mediaInterface->getPacketType(i);
			CSpeakerSourceInputServer *objSpeakerSourceInputServer = 
				m_mapSpeakerInputServer[strChannelName];

			std::shared_ptr<CSpeakerSourceChannel> objSpeakerSourceChannel(
				new CSpeakerSourceChannel(iChannelSN, strChannelName, 
					ePacketType,
					objSpeakerSourceInputServer,
					mediaInterface));
			if (objSpeakerSourceChannel->init())
			{
				addInterfaceChannel(strChannelName, objSpeakerSourceChannel);
			}

		}

		std::unique_lock<std::mutex> lock(m_mutexInterfaces);
		m_mapInterfaces[strName] = mediaInterface;
		return mxtrue;
	}

	mxbool CSpeakerSourceModule::delInterface(std::string strName)
	{
		std::unique_lock<std::mutex> lock(m_mutexInterfaces);
		m_mapInterfaces.erase(strName);
		return mxtrue;
	}

	mxbool CSpeakerSourceModule::addInterfaceChannel(std::string strName, 
		std::shared_ptr<CSpeakerSourceChannel> channel)
	{
		std::unique_lock<std::mutex> lock(m_mutexMediaInterfaceChn);
		m_mapSpeakerInterfacesChn[strName] = channel;
		m_mapSpeakerInterfacesChnProc[strName] = std::thread([channel]() {
    		channel->run();
		});

		return mxtrue;
	}

	mxbool CSpeakerSourceModule::delInterfaceChannel(std::string strName)
	{
		return mxbool();
	}

    CSpeakerSourceInputServer* CSpeakerSourceModule::getSpeakerSourceInputServer()
    {
        if(m_mapInterfaces.empty())
        {
            return NULL;
        }
        auto mediaInterface = m_mapInterfaces.begin()->second;
		int iChnNum = mediaInterface->getChnNum();

		if (iChnNum <= 0)
		{
			return NULL;
		}
        std::string strChannelName = mediaInterface->getChnName(0);
        return m_mapSpeakerInputServer.find(strChannelName)->second;
    }

	mxbool CSpeakerSourceModule::initServer()
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
						CSpeakerSourceRemoteEventServer * objSpeakerSourceRemoteEventServer =
							new CSpeakerSourceRemoteEventServer(this);

						if (!objSpeakerSourceRemoteEventServer)
							return mxfalse;

						objSpeakerSourceRemoteEventServer->init();

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
							objSpeakerSourceRemoteEventServer);

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
							strIP, iPort, strUnix, iLen))
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

	mxbool CSpeakerSourceModule::addSpeakerChannel(std::string strName,
		CSpeakerSourceInputServer * objSpeakerSourceInputServer)
	{
		std::unique_lock<std::mutex> lock(m_mutexSpeakerInputServer);
		m_mapSpeakerInputServer[strName] = objSpeakerSourceInputServer;
		return mxtrue;
	}

	std::string CSpeakerSourceModule::remoteEventServerProc(std::string strEvent, 
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
        else if (0 == strEvent.compare("SpeakerSourceUninit"))
        {
            return speakerSourceUninit();
        }
		else
		{
			return procResult(std::string("400"),"",
				strEvent.append("  event not support"));
		}
	}

	std::string CSpeakerSourceModule::open(std::string strParam)
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

		CSpeakerSourceInputServer* objSpeakerSourceChannel =
			m_mapSpeakerInputServer[strChannelName];

		if (objSpeakerSourceChannel)
		{
			return objSpeakerSourceChannel->open(strParam);
		}
		else
		{
			return procResult(std::string("400"),"",
				strChannelName.append(" not found"));
		}

		return procResult(std::string("200"), "",
			strChannelName.append("open success"));
	}

	std::string CSpeakerSourceModule::close(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerSourceModule::config(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerSourceModule::reset(std::string strParam)
	{
		return std::string();
	}

    std::string CSpeakerSourceModule::speakerSourceUninit()
    {
        for (auto interface : m_mapInterfaces)
        {
            if(interface.second && interface.second.use_count() != 0)
            {
                interface.second->unInit();
            }
        }
        return procResult("200","","speaker source uninit success");
    }

	std::string CSpeakerSourceModule::procResult(std::string code, 
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
