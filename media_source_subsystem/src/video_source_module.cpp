#include "video_source_module.h"
#include "media_interface_factory.h"
#include <list>
#include <thread>
#include "com_client_end_point.h"
#include "video_module_local_event_server.h"
#include "com_server_end_point.h"
#include "com_rpc_client_end_point.h"
#include "cJSON.h"
#include "com_rpc_server_end_point.h"
#include "video_source_remote_event_server.h"
#include <iostream>
#include "log_mx.h"

namespace maix {
	CVideoSourceModule::CVideoSourceModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
	{
		m_iLowpowerPrintCount = 0;
	}

	CVideoSourceModule::~CVideoSourceModule()
	{
	}

	mxbool CVideoSourceModule::init()
	{
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
			if (info.m_strType.compare("video") == 0)
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
							std::cout << "add interface failed: " <<
								strInterface << std::endl;
							logPrint(MX_LOG_ERROR, "add interface failed: %s", 
								strInterface.c_str());
						}
					}
					else
					{
						std::cout << "init interface failed: " <<
								strInterface << std::endl;
						logPrint(MX_LOG_ERROR, "init interface failed: %s", 
								strInterface.c_str());
					}
					
				}
				else
				{
					std::cout << "loadconfig interface failed: " <<
								strInterface << std::endl;
					logPrint(MX_LOG_ERROR, "loadconfig interface failed: %s", 
								strInterface.c_str());
				}
			}

		}

		if (!initConnectModule())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		if (!initLowPower())
			return mxfalse;

		return mxtrue;
	}

	mxbool CVideoSourceModule::unInit()
	{
		return mxbool();
	}

	mxbool CVideoSourceModule::addInterface(std::string strName, 
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

			std::shared_ptr<CVideoSourceInput> objVideoSourceInput(
				new CVideoSourceInput(iChannelSN, strChannelName, ePacketType,
					this, mediaInterface));

			if (!addInterfaceChannel(strChannelName, ePacketType, objVideoSourceInput))
					return mxfalse;
			
		}
		
		std::unique_lock<std::mutex> lock(m_mutexMediaInterfaces);
		m_mapMediaInterfaces[strName] = mediaInterface;
		return mxtrue;
	}

	mxbool CVideoSourceModule::delInterface(std::string strName)
	{
		std::unique_lock<std::mutex> lock(m_mutexMediaInterfaces);
		m_mapMediaInterfaces.erase(strName);
		return mxtrue;
	}

	mxbool CVideoSourceModule::addInterfaceChannel(std::string strName, E_P_TYPE ePacketType, 
		std::shared_ptr<CVideoSourceInput> input)
	{
		std::unique_lock<std::mutex> lock(m_mutexInterfaceChannel);
		std::shared_ptr<CVideoSourceChannel> channel(
			new CVideoSourceChannel(strName, ePacketType, this,this));

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

	mxbool CVideoSourceModule::delInterfaceChannel(std::string strName)
	{
		return mxbool();
	}

	mxbool CVideoSourceModule::initServer()
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
						if (!regClient(strName, comClientEndPoint))
						{
							return mxfalse;
						}

						std::shared_ptr<CVideoModuleLocalEventServer> serverHandle(
							new CVideoModuleLocalEventServer());

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
						CVideoSourceRemoteEventServer * objVideoSourceRemoteEventServer =
							new CVideoSourceRemoteEventServer(this);

						if (!objVideoSourceRemoteEventServer)
							return mxfalse;

						objVideoSourceRemoteEventServer->init();

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
							objVideoSourceRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}

		}

		return mxtrue;
	}

	mxbool CVideoSourceModule::initConnectModule()
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

	std::string CVideoSourceModule::remoteEventServerProc(std::string strEvent, 
		std::string strParam)
	{
		if (0 == strEvent.compare("openVideo"))
		{
			return open(strParam);
		}
		else if (0 == strEvent.compare("closeVideo"))
		{
			return close(strParam);
		}
		else if (0 == strEvent.compare("configVideo"))
		{
			return config(strParam);
		}
		else if (0 == strEvent.compare("resetVideo"))
		{
			return reset(strParam);
		}
		else if (0 == strEvent.compare("EnterLowPower"))
		{
			return enterLowPower(strParam);
		}
		else
		{
			return procResult(std::string("400"),"",
				strEvent.append("  event not support"));
		}

	}

	std::string CVideoSourceModule::open(std::string strParam)
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
        int iLiveStream = 0;
        int iEventStream = 0;

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

			cJSON *jsonLiveStream = cJSON_GetObjectItem(jsonRoot, "liveStream");
			if (!jsonLiveStream)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("liveStream param parse failed"));
			}
			else
			{
				iLiveStream = jsonLiveStream->valueint;
				system("touch /tmp/_liveing");
				if(iLiveStream == 1)
				{
					std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
                    if (m_mapChannelLiveStream.count(strServerName) > 0)
                    {
                        m_mapChannelLiveStream[strServerName].bLiveStream = 1;
                    }
                    else
                    {
                        T_CHANNEL_LIVE_INFO liveInfo;
                        liveInfo.bEventStream = 0;
                        liveInfo.bLiveStream = 1;
                        m_mapChannelLiveStream[strServerName] = liveInfo;
                    }
					logPrint(MX_LOG_INFOR, "m_iLiveStream add %s ",strServerName.c_str());
				}
			}

            cJSON *jsonEventStream = cJSON_GetObjectItem(jsonRoot, "eventStream");
			if (!jsonEventStream)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("eventStream param parse failed"));
			}
			else
			{
				iEventStream = jsonEventStream->valueint;

				if(iEventStream == 1)
				{
					std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
                    if (m_mapChannelLiveStream.count(strServerName) > 0)
                    {
                        m_mapChannelLiveStream[strServerName].bEventStream = 1;
                    }
                    else
                    {
                        T_CHANNEL_LIVE_INFO liveInfo;
                        liveInfo.bEventStream = 1;
                        liveInfo.bLiveStream = 0;
                        m_mapChannelLiveStream[strServerName] = liveInfo;
                    }
                    logPrint(MX_LOG_INFOR, "m_iEventStreamNum add: %s", strServerName.c_str());
				}
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
		std::shared_ptr<CVideoSourceChannel> objVideoSourceChannel =
			m_mapInterfaceChannel[strChannelName];

		if (objVideoSourceChannel)
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

			if (!objVideoSourceChannel->open(
				strModuleGUID, strServerName))
			{
				return procResult(std::string("500"),"",
					strChannelName.append("open failed"));
			}
            if(iEventStream == 1)
            {
                m_mapInterfaceInput[strServerName]->getJpegFrameData();
                // m_mapInterfaceInput[strServerName]->getIDRframe();
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

	std::string CVideoSourceModule::close(std::string strParam)
	{
	    cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strChannelName;
		std::string strServerName;
        int iLiveStream = 0;
        int iEventStream = 0;

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

			cJSON *jsonLiveStream = cJSON_GetObjectItem(jsonRoot, "liveStream");
			if (!jsonLiveStream)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("liveStream param parse failed"));
			}
			else
			{
				iLiveStream = jsonLiveStream->valueint;

				if(iLiveStream == 0)
				{
					std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
                    if (m_mapChannelLiveStream.count(strServerName) > 0)
                    {
                        m_mapChannelLiveStream[strServerName].bLiveStream = 0;
                    }
					logPrint(MX_LOG_INFOR, "m_iLiveStream del:%s ",strServerName.c_str());
				}
			}

            cJSON *jsonEventStream = cJSON_GetObjectItem(jsonRoot, "eventStream");
			if (!jsonEventStream)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"),"",
					std::string("eventStream param parse failed"));
			}
			else
			{
				iEventStream = jsonEventStream->valueint;

				if(iEventStream == 0)
				{
					std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
                    if (m_mapChannelLiveStream.count(strServerName) > 0)
                    {
                        m_mapChannelLiveStream[strServerName].bEventStream = 0;
                    }
                    logPrint(MX_LOG_INFOR, "m_iEventStreamNum:%s", strServerName.c_str());
				}
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
            logPrint(MX_LOG_ERROR,"video get module failed");
		}
		else
		{
			imodule->unRegClient(strServerName);
            // CModule *module = dynamic_cast<CModule *>(imodule.get());
		}

		{
			std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
			bool bLiveing = false;
			for (auto info : m_mapChannelLiveStream)
			{
				if(info.second.bLiveStream == 1)
				{
					bLiveing = true;
					break;
				}
			}
			if(bLiveing == false)
			{
				system("rm /tmp/_liveing");
			}
		}

		// CModule *module = dynamic_cast<CModule *>(imodule.get());

		if (m_mapInterfaceChannel.count(strChannelName) != 0)
		{
			std::shared_ptr<CVideoSourceChannel> objVideoSourceChannel =
				m_mapInterfaceChannel[strChannelName];
			if (objVideoSourceChannel)
			{
				std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
                
				if(m_mapChannelLiveStream.count(strServerName) > 0)
				{
                    if(m_mapChannelLiveStream[strServerName].bEventStream == 0 && m_mapChannelLiveStream[strServerName].bLiveStream == 0)
                    {
                        logPrint(MX_LOG_INFOR, "close video");
					    objVideoSourceChannel->close(strModuleGUID, strServerName);
                    }
				}
				else
                {
                    logPrint(MX_LOG_INFOR, "close video none map");
                    objVideoSourceChannel->close(strModuleGUID, strServerName);
                }
			}
			else
			{
				return  procResult(std::string("500"), "",
					strChannelName.append(" not exist"));
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

	std::string CVideoSourceModule::config(std::string strParam)
	{
		std::string strInterface;
        int iCount = 0;
        cJSON *jsonParam = cJSON_Parse(strParam.c_str());
        cJSON *jsonValue = cJSON_GetObjectItem(jsonParam, "interfaceName");
        if (jsonValue)
        {
            strInterface = std::string(jsonValue->valuestring);
        }
        std::unique_lock<std::mutex> lock(m_mutexMediaInterfaces);
        iCount = m_mapMediaInterfaces.count(strInterface);
        if(iCount != 0)
        {
            if(!m_mapMediaInterfaces[strInterface]->config(strParam))
            {
                return procResult(std::string("500"),"",std::string("load config failed"));
            }else{
                return procResult(std::string("200"),"", std::string("load config success"));
            }
        }else{
            return procResult(std::string("400"),"", std::string(" strInterface not found"));
        }
	}

	std::string CVideoSourceModule::reset(std::string strParam)
	{
		return std::string();
	}

	std::string CVideoSourceModule::enterLowPower(std::string strParam)
	{
		m_iLowpowerPrintCount++;
		std::unique_lock<std::mutex> lock(m_mutexLiveStreamNum);
        bool bLiveing = false;
        for (auto info : m_mapChannelLiveStream)
        {
            if(info.second.bLiveStream == 1)
            {
				if(m_iLowpowerPrintCount == 300)
				{
					logPrint(MX_LOG_INFOR, "live channel:%s", info.first.c_str());
				}
                if (m_mapInterfaceChannel.count(info.first) != 0)
                {
                    std::shared_ptr<CVideoSourceChannel> objVideoSourceChannel =
                        m_mapInterfaceChannel[info.first];
                    if (objVideoSourceChannel && objVideoSourceChannel->getMapClientNum() != 0)
                    {
                        bLiveing = true;
                        break;
                    }
                }
            }
        }
        if(m_iLowpowerPrintCount == 300)
		{
			logPrint(MX_LOG_INFOR, "Enter low power mode ,live stream:%d", bLiveing);
			m_iLowpowerPrintCount = 0;
		}
		if(bLiveing)
		{
            return procResult(std::string("500"), "", "video source is in live stream");
		}
		else
		{
			{
				std::unique_lock<std::mutex> lock(m_mutexLowPower);
				m_conditionLowPower.notify_one();
			}
            system("rm /tmp/_liveing");
			return procResult(std::string("200"), "", "video source enter low power");
		}	
	}

	std::string CVideoSourceModule::procResult(std::string code, 
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

	mxbool CVideoSourceModule::initLowPower()
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

	void CVideoSourceModule::lowPowerRun()
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
			usleep(1000*1000);
#endif
		}
	}

	std::string CVideoSourceModule::lowPowerProc()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();
		
		cJSON_AddStringToObject(jsonRoot, "event", "VideoSourceExit");
		
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		//todo add close to mediasource
		
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}    

}
