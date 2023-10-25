#include "audio_source_module.h"
#include "media_interface_factory.h"
#include <list>
#include <thread>
#include "com_client_end_point.h"
#include "audio_module_local_event_server.h"
#include "com_server_end_point.h"
#include "com_rpc_client_end_point.h"
#include "com_rpc_server_end_point.h"
#include "audio_source_remote_event_server.h"
#include "cJSON.h"
#include "log_mx.h"

namespace maix {
	CAudioSourceModule::CAudioSourceModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
	{
	}

	CAudioSourceModule::~CAudioSourceModule()
	{
	}

	mxbool CAudioSourceModule::init()
	{
		std::string strInterfaceConfigPath;
		if (!getConfig("INTERFACE_CONFIG", "PATH", strInterfaceConfigPath))
		{
			return mxfalse;
		}

		std::list<std::string> listInterfaces;
		listInterfaces = CMediaInterfaceFactory::interfaceList();
		std::list<std::string>::iterator itl;
		std::string strInterface;
		for(itl = listInterfaces.begin(); 
				itl != listInterfaces.end(); itl++)
		{
			strInterface = *itl;
			if (strInterface.empty())
			{
				continue;
			}

			MediaInterfaceInfo info =
				CMediaInterfaceFactory::getInfo(strInterface);
			if (info.m_strType.compare("audio") == 0)
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
					logPrint(MX_LOG_ERROR, "loadconfgi interface failed: %s", 
								strInterface.c_str());
					return mxfalse;
				}
			
			}

		}
		
		if (!initServer())
			return mxfalse;

		return mxtrue;
	}

	mxbool CAudioSourceModule::unInit()
	{
		return mxtrue;
	}

	mxbool CAudioSourceModule::addInterface(std::string strName, 
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

			std::shared_ptr<CAudioSourceInput> objAudioSourceInput(
				new CAudioSourceInput(iChannelSN, strChannelName, ePacketType,
					this,
					mediaInterface));
			
			if (!addInterfaceChannel(strChannelName, objAudioSourceInput))
				return mxfalse;
		}

		std::unique_lock<std::mutex> lock(m_mutexMediaInterfaces);
		m_mapMediaInterfaces[strName] = mediaInterface;

		return mxtrue;
	}

	mxbool CAudioSourceModule::delInterface(std::string strName)
	{
		std::unique_lock<std::mutex> lock(m_mutexMediaInterfaces);
		m_mapMediaInterfaces.erase(strName);
		return mxtrue;
	}

	mxbool CAudioSourceModule::addInterfaceChannel(std::string strName, 
		std::shared_ptr<CAudioSourceInput> input)
	{
		std::unique_lock<std::mutex> lock(m_mutexInterfaceChannel);
		std::shared_ptr<CAudioSourceChannel> channel(
			new CAudioSourceChannel(strName, this));
		if (!channel->init())
			return mxfalse;

		if (!input->init(channel))
		{
			return mxfalse;
		}

		m_mapInterfaceChannel[strName] = channel;
		m_mapInterfaceChannelProc[strName] = std::thread([channel]() {
			channel->run();
		});

		m_mapInterfaceInput[strName] = input;
		m_mapInterfaceInputProc[strName] = std::thread([input]() {
			input->run();
		});

		return mxtrue;
	}

	mxbool CAudioSourceModule::delInterfaceChannel(std::string strName)
	{
		return mxbool();
	}

	mxbool CAudioSourceModule::initServer()
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
						if (!regClient(strName, comClientEndPoint))
						{
							return mxfalse;
						}

						std::shared_ptr<CAudioModuleLocalEventServer> serverHandle(
							new CAudioModuleLocalEventServer());

						std::shared_ptr<CComServerEndPoint> comServerEndPoint(
							new CComServerEndPoint(this));
						comServerEndPoint->init(serverHandle);

						if (!regServer(strName, comServerEndPoint))
						{
							return mxfalse;
						}
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
						CAudioSourceRemoteEventServer * objAudioSourceRemoteEventServer =
							new CAudioSourceRemoteEventServer(this);

						if (!objAudioSourceRemoteEventServer)
							return mxfalse;

						objAudioSourceRemoteEventServer->init();

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
							objAudioSourceRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}

		}

		return mxtrue;
	}

	std::string CAudioSourceModule::remoteEventServerProc(std::string strEvent, 
		std::string strParam)
	{
		if (0 == strEvent.compare("openAudio"))
		{
			return open(strParam);
		}
		else if (0 == strEvent.compare("closeAudio"))
		{
			return close(strParam);
		}
		else if (0 == strEvent.compare("configAudio"))
		{
			return config(strParam);
		}
		else if (0 == strEvent.compare("resetAudio"))
		{
			return reset(strParam);
		}
		else
		{
			return procResult(std::string("400"),"",
				strEvent.append("  event not support"));
		}
	}

	std::string CAudioSourceModule::open(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strModuleName;
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
		std::shared_ptr<CAudioSourceChannel> objAudioSourceChannel =
			m_mapInterfaceChannel[strChannelName];

		if (objAudioSourceChannel)
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

			if (!objAudioSourceChannel->open(
				strModuleGUID, strServerName))
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

	std::string CAudioSourceModule::close(std::string strParam)
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
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
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

		// CModule *module = dynamic_cast<CModule *>(imodule.get());

		if (m_mapInterfaceChannel.count(strChannelName) != 0)
		{
			std::shared_ptr<CAudioSourceChannel> objAudioSourceChannel =
				m_mapInterfaceChannel[strChannelName];
			if (objAudioSourceChannel)
			{
				objAudioSourceChannel->close(strModuleGUID, strServerName);
			}
			else
			{
				// return  procResult(std::string("500"), "",
				// 	strChannelName.append(" not exist"));
                logPrint(MX_LOG_ERROR,"not exist");
			}

		}
		else
		{
			return procResult(std::string("500"), "",
				strChannelName.append(" not exist"));
		}

		return procResult(std::string("200"), "",
			strChannelName.append(" close success"));
	}

	std::string CAudioSourceModule::config(std::string strParam)
	{
		return std::string();
	}

	std::string CAudioSourceModule::reset(std::string strParam)
	{
		return std::string();
	}

	std::string CAudioSourceModule::procResult(std::string code,
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
