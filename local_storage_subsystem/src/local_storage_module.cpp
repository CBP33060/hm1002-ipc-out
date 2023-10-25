#include "local_storage_module.h"
#include "com_rpc_server_end_point.h"
#include "local_storage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#include "common.h"
#include "log_mx.h"

#define MEDIA_FILE_PATH "C:/sunyang/workplace/IPC-SYSTEM/IPC-OUT"

namespace maix {
	CLocalStorageModule::CLocalStorageModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
	{
	}

	CLocalStorageModule::~CLocalStorageModule()
	{
	}

	mxbool CLocalStorageModule::init()
	{
		if (!initConnectModule())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		if (!initLowPower())
			return mxfalse;

		if (!initdOffLineEvent())
			return mxfalse;

		return mxtrue;
	}

	mxbool CLocalStorageModule::unInit()
	{
		return mxbool();
	}

	mxbool CLocalStorageModule::initServer()
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
						CLocalStorageRemoteEventServer * objLocalStorageRemoteEventServer =
							new CLocalStorageRemoteEventServer(this);

						if (!objLocalStorageRemoteEventServer)
							return mxfalse;

						objLocalStorageRemoteEventServer->init();

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
							objLocalStorageRemoteEventServer);

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

					if ((strName.compare("video_channel_1_server") == 0) ||
						(strName.compare("video_channel_2_server") == 0) ||
						(strName.compare("video_channel_3_server") == 0) ||
						(strName.compare("video_channel_4_server") == 0))
						// (strName.compare("video_channel_5_server") == 0))
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

						std::shared_ptr<CComRcfServerEndPoint> objServerEndPoint(
							new CComRcfServerEndPoint(this));

						objServerEndPoint->init(serverConfig, objAudioSourceInputServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;

						if (!addAudioChannel(strName,
							objAudioSourceInputServer))
						{
							return mxfalse;
						}

					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CLocalStorageModule::initConnectModule()
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

	mxbool CLocalStorageModule::initLowPower()
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

	mxbool CLocalStorageModule::initdOffLineEvent()
	{
		std::shared_ptr<COffLineEventProc> offLineEventProc(
			new COffLineEventProc(this));

		if (!offLineEventProc->init(20))
			return mxfalse;

		m_threadOffLineEvent = std::thread([offLineEventProc]() {
			offLineEventProc->run();
		});

		m_offLineEventProc = offLineEventProc;
		return mxtrue;
	}

	mxbool CLocalStorageModule::initUploadOfflineMediaFile()
	{
		std::shared_ptr<CUploadOfflineMediaFile> objUploadOfflineMediaFile(
			new CUploadOfflineMediaFile(this));
		
		if (!objUploadOfflineMediaFile->init())
			return mxfalse;

		m_threadUploadOfflineMediaFile = std::thread([objUploadOfflineMediaFile]() {
			objUploadOfflineMediaFile->run();
		});

		m_uploadOfflineMediaFile = objUploadOfflineMediaFile;
		return mxtrue;
	}

	void CLocalStorageModule::lowPowerRun()
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

	std::string CLocalStorageModule::lowPowerProc()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "VideoManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

	std::string CLocalStorageModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		if (0 == strEvent.compare("EnterLowPower"))
		{
			return enterLowPower(strParam);
		}
		else if (0 == strEvent.compare("EventOccurrence"))
		{
			return eventOccurrence(strParam);
		}
		else if (0 == strEvent.compare("GetMediaList"))
		{
			return getMediaList(strParam);
		}
		else if (0 == strEvent.compare("GetMediaFileStart"))
		{
			return getMediaFileStart(strParam);
		}
		else if (0 == strEvent.compare("GetMediaFileStop"))
		{
			return getMediaFileStop(strParam);
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	mxbool CLocalStorageModule::addVideoChannel(
		std::string strName, CVideoSourceInputServer * objVideoSourceInputServer)
	{
		std::unique_lock<std::mutex> lock(m_mutexVideoChn);
		std::shared_ptr<CVideoManageChannel> channel(
			new CVideoManageChannel(this, strName, objVideoSourceInputServer));
		if (!channel->init())
			return mxfalse;

		m_mapVideoChn[strName] = channel;
		m_mapVideoChnProc[strName] = std::thread([channel]() {
			channel->run();
		});

		std::shared_ptr<CVideoManageChannelSession> channelSession(
			new CVideoManageChannelSession(objVideoSourceInputServer));

		if (!channelSession->init())
		{
			return mxfalse;
		}

		m_mapVideoChnSession[strName] = channelSession;
		m_mapVideoChnSessionProc[strName] = std::thread([channelSession]() {
			channelSession->run();
		});

		return mxtrue;
	}

	mxbool CLocalStorageModule::addAudioChannel(std::string strName, 
		CAudioSourceInputServer * objAudioSourceInputServer)
	{
		std::unique_lock<std::mutex> lock(m_mutexAudioChn);
		std::shared_ptr<CAudioManageChannel> channel(
			new CAudioManageChannel(this, strName, objAudioSourceInputServer));
		if (!channel->init())
			return mxfalse;

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

	std::string CLocalStorageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		return procResult(std::string("200"), "",
			"video manage enter low power");
	}

	std::string CLocalStorageModule::eventOccurrence(
		std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexVideoChn);
			std::map <std::string,
				std::shared_ptr<CVideoManageChannelSession>>::iterator iter;

			for (iter = m_mapVideoChnSession.begin();
			iter != m_mapVideoChnSession.end(); iter++)
			{
				E_STATE state = iter->second->getState();
				if(state != E_OPENED)
					iter->second->setState(E_OPENING);
			}
			
		}
		
		{
			std::unique_lock<std::mutex> lock(m_mutexAudioChn);
			std::map < std::string,
				std::shared_ptr < CAudioManageChannelSession >>::iterator iter;

			for (iter = m_mapAudioChnSession.begin();
			iter != m_mapAudioChnSession.end(); iter++)
			{
				E_A_STATE state = iter->second->getState();
				if (state != E_A_OPENED)
					iter->second->setState(E_A_OPENING);
			}
		}

		m_offLineEventProc->startRecord();

		return std::string();
	}

	std::string CLocalStorageModule::getMediaList(std::string strParam)
	{
		std::vector<std::string> files;
		mxbool bRet = mxfalse;
		bRet = getFiles(MEDIA_FILE_PATH, files, ".mp4");

		if (bRet)
		{
			cJSON *jsonRoot = cJSON_CreateObject();
			cJSON *jsonArray = cJSON_CreateArray();
			for (int i = 0; i < files.size(); ++i)
			{
				logPrint(MX_LOG_INFOR, "%s", files[i].c_str());
				cJSON_AddItemToArray(jsonArray, 
					cJSON_CreateString(files[i].c_str()));
			}
			cJSON_AddItemToObject(jsonRoot, "MediaList", jsonArray);
			std::string strMediaList = cJSON_Print(jsonRoot);
			logPrint(MX_LOG_INFOR, "%s", strMediaList.c_str());
			cJSON_Delete(jsonRoot);

			return  procResult(std::string("200"), strMediaList,
				std::string("success"));
		}
		else
		{
			return  procResult(std::string("500"), "",
				std::string("error"));
		}
	}

	std::string CLocalStorageModule::getMediaFileStart(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strModuleName;
		std::string strServerName;
		std::string strFileName;
		std::string strKey;
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
				return procResult(std::string("500"), "",
					std::string("serverName param parse failed"));
			}
			else
			{
				strServerName = std::string(jsonServerName->valuestring);
			}

			cJSON *jsonKey = cJSON_GetObjectItem(jsonRoot, "key");
			if (!jsonKey)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("key param parse failed"));
			}
			else
			{
				strKey = std::string(jsonKey->valuestring);
			}

			cJSON *jsonFileName = cJSON_GetObjectItem(jsonRoot, "fileName");
			if (!jsonFileName)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("channelID param parse failed"));
			}
			else
			{
				strFileName = std::string(jsonFileName->valuestring);
			}

			cJSON *jsonType = cJSON_GetObjectItem(jsonRoot, "type");
			if (!jsonType)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
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
				return procResult(std::string("500"), "",
					strServerName.append("create module failed"));
			}

		}

		CModule *module = dynamic_cast<CModule *>(imodule.get());
		
		if (m_uploadOfflineMediaFile)
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

			objClientEndPoint->init(clientConfig, E_CLIENT_FRAME);

			if (!module->regClient(strServerName, objClientEndPoint))
			{
				return procResult(std::string("500"), "",
					std::string("client reg failed"));
			}

			if (!m_uploadOfflineMediaFile->open(
				strModuleGUID, strServerName, strKey, strFileName))
			{
				return procResult(std::string("500"), "",
					strServerName.append("open failed"));
			}
		}
		else
		{
			return procResult(std::string("400"), "",
				strServerName.append(" not found"));
		}

		return procResult(std::string("200"), "",
			strServerName.append("open success"));
	}

	std::string CLocalStorageModule::getMediaFileStop(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strModuleName;
		std::string strServerName;
		
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
				return procResult(std::string("500"), "",
					std::string("serverName param parse failed"));
			}
			else
			{
				strServerName = std::string(jsonServerName->valuestring);
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
				return procResult(std::string("500"), "",
					strServerName.append("create module failed"));
			}

		}

		CModule *module = dynamic_cast<CModule *>(imodule.get());

		if (m_uploadOfflineMediaFile)
		{
			if (!m_uploadOfflineMediaFile->close(
				strModuleGUID, strServerName))
			{
				return procResult(std::string("500"), "",
					strServerName.append("close failed"));
			}
		}
		else
		{
			return procResult(std::string("400"), "",
				strServerName.append(" not found"));
		}

		return procResult(std::string("200"), "",
			strServerName.append("close success"));
	}

	mxbool CLocalStorageModule::stopRecord()
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexVideoChn);
			std::map <std::string,
				std::shared_ptr<CVideoManageChannelSession >> ::iterator iter;

			for (iter = m_mapVideoChnSession.begin();
			iter != m_mapVideoChnSession.end(); iter++)
			{
				iter->second->setState(E_CLOSEING);
			}

		}

		{
			std::unique_lock<std::mutex> lock(m_mutexAudioChn);
			std::map < std::string,
				std::shared_ptr < CAudioManageChannelSession >> ::iterator iter;

			for (iter = m_mapAudioChnSession.begin();
			iter != m_mapAudioChnSession.end(); iter++)
			{
				iter->second->setState(E_A_CLOSEING);
			}
		}

		return mxtrue;
	}

	std::string CLocalStorageModule::procResult(
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

	bool CLocalStorageModule::pushFrameDataToEvent(
		std::shared_ptr<CMediaFramePacket>& packet)
	{
		return m_offLineEventProc->pushFrameData(packet);
	}
}