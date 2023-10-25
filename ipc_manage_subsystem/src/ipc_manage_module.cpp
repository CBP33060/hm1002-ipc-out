#include "ipc_manage_module.h"
#include <list>
#include <thread>
#include "com_client_end_point.h"
#include "com_server_end_point.h"
#include "com_rpc_client_end_point.h"
#include "com_rpc_server_end_point.h"
#include "ipc_manage_remote_event_server.h"
#include "ipc_manage_bind_state.h"
#include "cJSON.h"
#include "ipc_manage_access.h"
#include "crypt_api_mx.h"
#include "mbedtls/base64.h"
#include "log_mx.h"
#include "common.h"
#include "fw_env_para.h"

namespace maix {
	CIPCManageModule::CIPCManageModule(std::string strGUID, 
		std::string strName)
		: CModule(strGUID, strName)
	{
		memset(m_acAESKey, 0, sizeof(m_acAESKey));
		
	}

	CIPCManageModule::~CIPCManageModule()
	{
		unInit();
	}

	mxbool CIPCManageModule::init()
	{
		m_strDID = getDID();

		if (!initAccess())
			return mxfalse;

		if (!initConnectModule())
			return mxfalse;

		if (!initMcuModuleServer())
			return mxfalse;

		// usleep(10*1000);
		// std::unique_lock<std::mutex> lock(m_mutexSendAes);
		// m_conditionSendAes.notify_one();

		if (!initEventModuleServer())
			return mxfalse;

		if (!initDevModuleServer())
			return mxfalse;

		if (!initAudioModuleServer())
			return mxfalse;

		if (!initVideoModuleServer())
			return mxfalse;

		if (!initSpeakerModuleServer())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		if (!initLowPower())
			return mxfalse;

		// if (!initLocalStorage())
		// 	return mxfalse;

		if (!initCenterSubsystem())
			return mxfalse;

		if (!initBindState())
			return mxfalse;

		if (!initOTA())
			return mxfalse;

        if (!initLogManage())
            return mxfalse;
			
		return mxtrue;
	}

	mxbool CIPCManageModule::unInit()
	{
		if (m_objIPCManageAccess)
		{
			delete m_objIPCManageAccess;
			m_objIPCManageAccess = NULL;
		}

		if (m_objIPCManageBindState)
		{
			delete m_objIPCManageBindState;
			m_objIPCManageBindState = NULL;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initServer()
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

                        if ((iType & COM_PROXY_S_TYPE_TCP) == COM_PROXY_S_TYPE_TCP)
						{
							CIPCManageRemoteEventServer * objIPCManageRemoteEventServer =
										new CIPCManageRemoteEventServer(this);
							T_COM_PROXY_SERVER_CONFIG serverConfig;
							serverConfig.m_iType = 1;
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
								objIPCManageRemoteEventServer);

							if (!regServer(strName, objServerEndPoint))
								return mxfalse;
						}

						if ((iType & COM_PROXY_S_TYPE_UDP) == COM_PROXY_S_TYPE_UDP)
						{

						}
						
						if ((iType & COM_PROXY_S_TYPE_UNIX) == COM_PROXY_S_TYPE_UNIX)
						{
							CIPCManageRemoteEventServer * objIPCManageRemoteEventServer =
										new CIPCManageRemoteEventServer(this);
							T_COM_PROXY_SERVER_CONFIG serverConfig;
							serverConfig.m_iType = 4;
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
								objIPCManageRemoteEventServer);
							std::string strNameUnix = strName;
							strNameUnix.append("_unix");
							if (!regServer(strNameUnix, objServerEndPoint))
								return mxfalse;
						}
					}
				}
			}

		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initConnectModule()
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

	mxbool CIPCManageModule::initLowPower()
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

	void CIPCManageModule::lowPowerRun()
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

	std::string CIPCManageModule::lowPowerProc()
	{
        sendMikeEnterLowpower();
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "IPCManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

    mxbool CIPCManageModule::sendMikeEnterLowpower()
    {
        std::string strResult;
        cJSON *jsonRoot = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "EnterLowpower");

        char *out = cJSON_Print(jsonRoot);
        std::string strEventConfig = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        int ret = 0;
        size_t iAESParmLen = 0;
        char *acAESConfig = (char *)malloc(2 * strEventConfig.length());
        if (acAESConfig == NULL)
            return mxfalse;

        memset(acAESConfig, 0, 2 * strEventConfig.length());
        ret = crypto_aes128_encrypt_base64(m_acAESKey,
            (unsigned char*)strEventConfig.c_str(),
            strEventConfig.length(),
            (unsigned char*)acAESConfig,
            2 * strEventConfig.length(),
            &iAESParmLen);
        if(ret < 0)
        {
            free(acAESConfig);
            return mxfalse;
        }

        strResult = output(m_strIPCAgentGUID, m_strIPCAgentRemoteEventServer,
            (unsigned char*)acAESConfig, iAESParmLen);
        free(acAESConfig);

        unsigned char pcDecryptData[2048] = { 0 };
        int iDecryptDataLen = 0;
        if (crypto_aes128_decrypt_base64(m_acAESKey,
            (unsigned char*)strResult.c_str(),
            strResult.length(),
            pcDecryptData, &iDecryptDataLen) != 0)
            return mxfalse;

        std::string strDecryptResult =
            std::string((char*)pcDecryptData, iDecryptDataLen);

        return mxtrue;
    }

	mxbool CIPCManageModule::initMcuModuleServer()
	{
		if (!getConfig("MCU_REMOTE_EVENT", "GUID", m_strMcuModuleGUID))
		{
			return mxfalse;
		}

		if (!getConfig("MCU_REMOTE_EVENT", "SERVER", m_strMcuModuleRemoteServer))
		{
			return mxfalse;
		}

		m_threadSendMcuAes = std::thread([this]() {
			this->sendMcuAesRun();
		});

		return mxtrue;
	}

	void CIPCManageModule::sendMcuAesRun()
	{
		mxbool bSend = mxfalse;
		while (1)
		{
			std::unique_lock<std::mutex> lock(m_mutexSendAes);
			m_conditionSendAes.wait(lock);
			
			bSend = mxtrue;
			while(bSend)
			{
				std::string procResult = sendAesProc();
				std::string strResult = output(m_strMcuModuleGUID,
					m_strMcuModuleRemoteServer,
					(unsigned char*)procResult.c_str(), procResult.length());

				if (strResult.length() > 0)
				{
					std::string strCode;
					cJSON *jsonRoot = cJSON_Parse(strResult.c_str());
					if (jsonRoot)
					{
						cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
						if (jsonCode)
						{
							strCode = std::string(jsonCode->valuestring);
						}
					}

					if (strCode.compare("200") == 0)
					{
						logPrint(MX_LOG_DEBUG, "xxxx send mcu aes success");
						bSend = mxfalse;
						break;
					}
				}
#ifdef _WIN32
				Sleep(1000);
#else
				usleep(1000 * 1000);
#endif
			}

#ifdef _WIN32
			Sleep(1000);
#else
			usleep(1000 * 1000);
#endif
		}
	}

	std::string CIPCManageModule::sendAesProc()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		int keyLen = sizeof(m_acAESKey) / sizeof(m_acAESKey[0]);
		size_t base64KeyLen = 0;
    	mbedtls_base64_encode(nullptr, 0, &base64KeyLen, m_acAESKey, keyLen);
		char base64Key[base64KeyLen + 1];
		memset(base64Key, 0, sizeof(base64Key));
		int ret = mbedtls_base64_encode((unsigned char*)base64Key, base64KeyLen, &base64KeyLen, m_acAESKey, keyLen);
		if (ret == 0) {
			logPrint(MX_LOG_INFOR, "after Base64=[%s]\r\n", base64Key);
		} else {
			logPrint(MX_LOG_INFOR, "Error: Base64 解码失败，错误码为:[%d]\r\n",ret);
			return procResult(std::string("500"), "","ipc manage Base64 Error");
    	}

		cJSON_AddStringToObject(jsonRoot, "event", "ipcManageEvent");
		cJSON_AddStringToObject(jsonParam, "aesValue", base64Key);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strIPCEvent = std::string(out);
		logPrint(MX_LOG_INFOR, "strIPCEvent=[%s]\r\n", strIPCEvent.c_str());
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strIPCEvent;
	}

	mxbool CIPCManageModule::initAccess()
	{
		CIPCManageAccess *objIPCManageAccess = new CIPCManageAccess(this);
		if (!objIPCManageAccess)
			return mxfalse;
		if (!objIPCManageAccess->init())
		{
			delete objIPCManageAccess;
			return mxfalse;
		}
			
		m_mapAccessSessionProc = std::thread([objIPCManageAccess]() {
			objIPCManageAccess->run();
		});
		m_objIPCManageAccess = objIPCManageAccess;

		return mxtrue;
	}

	mxbool CIPCManageModule::initBindState()
	{
		CIPCManageBindState *objIPCManageBindState = new CIPCManageBindState(this, m_strMcuModuleGUID, m_strMcuModuleRemoteServer);
		if (!objIPCManageBindState)
			return mxfalse;

		if (objIPCManageBindState->getBindState())
		{
			delete objIPCManageBindState;
			return mxtrue;
		}

		if (!objIPCManageBindState->init())
		{
			delete objIPCManageBindState;
			return mxfalse;
		}
			
		m_mapBindStateProc = std::thread([objIPCManageBindState]() {
			objIPCManageBindState->run();
		});

		m_objIPCManageBindState = objIPCManageBindState;

		return mxtrue;
	}

	mxbool CIPCManageModule::initLocalStorage()
	{
		if (!getConfig("LOCAL_STORAGE_REMOTE_EVENT", "GUID",
			m_strLocalStorageGUID))
		{
			return mxfalse;
		}

		if (!getConfig("LOCAL_STORAGE_REMOTE_EVENT", "SERVER",
			m_strLocalStorageRemoteEventServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initCenterSubsystem()
	{
		if (!getConfig("LOW_POWER_REMOTE_EVENT", "GUID",
			m_strCenterSubSystemGUID))
		{
			return mxfalse;
		}

		if (!getConfig("LOW_POWER_REMOTE_EVENT", "SERVER",
			m_strCenterSubSystemServer))
		{
			return mxfalse;
		}
		return mxtrue;
	}
	
	mxbool CIPCManageModule::initOTA()
	{
		if (!getConfig("OTA_REMOTE_EVENT", "GUID", m_strOTAGUID))
		{
			return mxfalse;
		}

		if (!getConfig("OTA_REMOTE_EVENT", "SERVER",
			m_strOTARemoteEventServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

    mxbool CIPCManageModule::initLogManage()
    {
        if (!getConfig("LOG_REMOTE_EVENT", "GUID", m_strLOGGUID))
        {
            return mxfalse;
        }

        if (!getConfig("LOG_REMOTE_EVENT", "SERVER",
            m_strLOGRemoteEventServer))
        {
            return mxfalse;
        }

        return mxtrue;
    }
	
	std::string CIPCManageModule::remoteEventServerProc(std::string strEvent,
		std::string strParam)
	{
		std::string strResult;
		if (strEvent.compare("IPCAgentConfig") == 0)
		{
			strResult = ipcAgentConfig(strParam);
		}
		else if (strEvent.compare("EventManageModule") == 0)
		{
			strResult = output(m_strEventModuleGUID, 
				m_strEventModuleRemoteServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("devConfig") == 0)
		{
			strResult = output(m_strDevModuleGUID, 
				m_strDevModuleRemoteServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("openAudio") == 0 ||
			strEvent.compare("closeAudio") == 0)
		{
			strResult = output(m_strAudioModuleGUID, 
				m_strAudioModuleRemoteServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("openVideo") == 0 ||
			strEvent.compare("closeVideo") == 0)
		{
			strResult =  output(m_strVideoModuleGUID, 
				m_strVideoModuleRemoteServer,
				(unsigned char*)strParam.c_str(), strParam.length());
            //打开视频流时，向center模块发送录像时长
            output(m_strLowPowerGUID,m_strLowPowerServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("openSpeaker") == 0)
		{
			strResult = output(m_strSpeakerModuleGUID, 
				m_strSpeakerModuleRemoteServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("EnterLowPower") == 0)
		{
			strResult = enterLowPower(strParam);
		}
		else if (strEvent.compare("AlarmEvent") == 0)
		{
			strResult = sendEventToIPC(strEvent, strParam);
		}
		else if (strEvent.compare("AlertEvent") == 0)
		{
			strResult = sendEventToIPC(strEvent, strParam);
		}
		else if (strEvent.compare("OTAProcess") == 0)
		{
			strResult = sendOTAStatusToIPC(strEvent, strParam);
		}
		else if(strEvent.compare("bindOK") == 0)
		{
			m_objIPCManageBindState->setBindState(E_BIND_SUCCESS);
			strResult = procResult(std::string("200"), "", "ipc bind ok");
		}
		else if(strEvent.compare("Unbind") == 0)
		{
			if (!unBind())
			{
				strResult = procResult(std::string("400"), "",
				strEvent.append("ipc unbind failed"));
			}
			else
			{
				strResult = procResult(std::string("200"), "", "ipc unbind ok");
			}
			
		}
		else if(strEvent.compare("GetMediaList") == 0 ||
		strEvent.compare("GetMediaFileStart") == 0 ||
		strEvent.compare("GetMediaFileStop") == 0)
		{
			strResult = output(m_strLocalStorageGUID,
				m_strLocalStorageRemoteEventServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("StartOTA") == 0 ||
			strEvent.compare("OTAData") == 0 ||
			strEvent.compare("SendOTAEnd") == 0)
		{
			strResult = output(m_strOTAGUID,
				m_strOTARemoteEventServer,
				(unsigned char*)strParam.c_str(), strParam.length());
		}
		else if (strEvent.compare("IPCAlertEvent") == 0)
		{
			enterLowPower(strParam);
		}
		else if(strEvent.compare("sendSpecData") == 0)
		{
			return sendSpecData(strEvent, strParam);
		}
        else if (strEvent.compare("StartUploadLog") == 0)
        {
            strResult = output(m_strLOGGUID,
                m_strLOGRemoteEventServer,
                (unsigned char*)strParam.c_str(), strParam.length());   
        }
        else if (strEvent.compare("LogData") == 0 ||
            strEvent.compare("LogDataEnd") == 0)
        {
            strResult = sendLogDataToIPC(strEvent,strParam);
        }
		else
		{
			strResult = procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}

		return strResult;
	}

	mxbool CIPCManageModule::getAllDevInfo()
	{
		std::string strOutputData;
		std::string strOutputResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "event", "GetDevInfo");
		cJSON_AddStringToObject(jsonRoot, "param", "");
		strOutputData = cJSON_PrintUnformatted(jsonRoot);
		strOutputResult = output(m_strMcuModuleGUID, m_strMcuModuleRemoteServer, (unsigned char*)strOutputData.c_str(), strOutputData.length());

		if(strOutputResult.length() > 0)
		{
			sendAllIPCDevInfo("sendIpcDevToCloud", strOutputResult.c_str());
		}
		else
		{
			cJSON_Delete(jsonRoot);
			return mxfalse;
		}
		cJSON_Delete(jsonRoot);
		return mxtrue;
	}

	mxbool CIPCManageModule::initEventModuleServer()
	{
		if (!getConfig("EVENT_REMOTE_SERVER", "GUID", m_strEventModuleGUID))
		{
			return mxfalse;
		}

		if (!getConfig("EVENT_REMOTE_SERVER", "SERVER", m_strEventModuleRemoteServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initDevModuleServer()
	{
		if (!getConfig("DEV_REMOTE_SERVER", "GUID", m_strDevModuleGUID))
		{
			return mxfalse;
		}

		if (!getConfig("DEV_REMOTE_SERVER", "SERVER", m_strDevModuleRemoteServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initAudioModuleServer()
	{
		if (!getConfig("AUDIO_REMOTE_SERVER", "GUID", m_strAudioModuleGUID))
		{
			return mxfalse;
		}

		if (!getConfig("AUDIO_REMOTE_SERVER", "SERVER", m_strAudioModuleRemoteServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initVideoModuleServer()
	{
		if (!getConfig("VIDEO_REMOTE_SERVER", "GUID", m_strVideoModuleGUID))
		{
			return mxfalse;
		}

		if (!getConfig("VIDEO_REMOTE_SERVER", "SERVER", m_strVideoModuleRemoteServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::initSpeakerModuleServer()
	{
		if (!getConfig("SPEAKER_REMOTE_SERVER", "GUID", m_strSpeakerModuleGUID))
		{
			return mxfalse;
		}

		if (!getConfig("SPEAKER_REMOTE_SERVER", "SERVER", m_strSpeakerModuleRemoteServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageModule::setAES128Key(unsigned char * pcAESKey)
	{
		if (pcAESKey == NULL)
			return mxfalse;
		memcpy(m_acAESKey, pcAESKey, sizeof(m_acAESKey));
		for (int i = 0; i < (int)sizeof(m_acAESKey); i++)
		{
			printf("%d ", m_acAESKey[i]);
		}
		printf("\n");
		std::unique_lock<std::mutex> lock(m_mutexSendAes);
		m_conditionSendAes.notify_one();
		return mxtrue;
	}

	mxbool CIPCManageModule::getAES128Key(unsigned char * pcAESKey)
	{
		if (pcAESKey == NULL)
			return mxfalse;
		memcpy(pcAESKey, m_acAESKey, sizeof(m_acAESKey));
		return mxtrue;
	}

	std::string CIPCManageModule::procResult(std::string code,
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

	std::string CIPCManageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		if(m_objIPCManageAccess->getAccessState() != E_ACCESS_EXIT)
			return procResult(std::string("500"), "",
				"ipc manage enter low power");
		else 
			return procResult(std::string("200"), "",
				"ipc manage enter low power");
	}

	std::string CIPCManageModule::sendEventToIPC(std::string strEvent, 
		std::string strParam)
	{
		setFWParaConfig("pir_false_wakeup_count", "0", 1);
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_Parse(strParam.c_str());

		cJSON_AddStringToObject(jsonRoot, "event", strEvent.c_str());
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strEventConfig = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		int ret = 0;
		size_t iAESParmLen = 0;
		char *acAESConfig = (char *)malloc(2 * strEventConfig.length());
		if (acAESConfig == NULL)
			return procResult(std::string("500"), "",
				"acAESConfig malloc error");

		memset(acAESConfig, 0, 2 * strEventConfig.length());
		ret = crypto_aes128_encrypt_base64(m_acAESKey,
			(unsigned char*)strEventConfig.c_str(),
			strEventConfig.length(),
			(unsigned char*)acAESConfig,
			2 * strEventConfig.length(),
			&iAESParmLen);
        if(ret < 0)
        {
            free(acAESConfig);
            return procResult(std::string("500"), "",
                "acAESConfig crypto error");
        }

		strResult = output(m_strIPCAgentGUID, m_strIPCAgentRemoteEventServer,
			(unsigned char*)acAESConfig, iAESParmLen);
		logPrint(MX_LOG_DEBUG, "acAESConfig:%s, ret:%s", acAESConfig, strResult.c_str());
		free(acAESConfig);

		if (strResult.length() > 0)
		{
			unsigned char pcDecryptData[2048] = { 0 };
			int iDecryptDataLen = 0;
			if (crypto_aes128_decrypt_base64(m_acAESKey,
				(unsigned char*)strResult.c_str(),
				strResult.length(),
				pcDecryptData, &iDecryptDataLen) != 0)
				return std::string("");

			std::string strDecryptResult =
				std::string((char*)pcDecryptData, iDecryptDataLen);

			logPrint(MX_LOG_DEBUG, "strDecryptResult: %s", strDecryptResult.c_str());
			return strDecryptResult;
		}
		else
		{
			// logPrint(MX_LOG_DEBUG, "send to local storage");
			// return sendEventToLocalStorage(strEvent, strParam);
            //产品需求变更，暂时不需要做离线缓存
            logPrint(MX_LOG_DEBUG, "send ipc err dev offline");
			return procResult(std::string("500"), "",
				"ipc offline");
		}
	}

    void CIPCManageModule::sendToIpcGetbindState()
    {
        logPrint(MX_LOG_INFOR,"sendToIpcGetbindState   ");
        std::string strResult;
        cJSON *jsonRoot = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "GetBindState");

        char *out = cJSON_Print(jsonRoot);
        std::string strEventConfig = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        int ret = 0;
        size_t iAESParmLen = 0;
        char *acAESConfig = (char *)malloc(2 * strEventConfig.length());
        if (acAESConfig == NULL)
            return ;

        memset(acAESConfig, 0, 2 * strEventConfig.length());
        ret = crypto_aes128_encrypt_base64(m_acAESKey,
            (unsigned char*)strEventConfig.c_str(),
            strEventConfig.length(),
            (unsigned char*)acAESConfig,
            2 * strEventConfig.length(),
            &iAESParmLen);
        if(ret < 0)
        {
            free(acAESConfig);
            return ;
        }

        strResult = output(m_strIPCAgentGUID, m_strIPCAgentRemoteEventServer,
            (unsigned char*)acAESConfig, iAESParmLen);
        free(acAESConfig);

        unsigned char pcDecryptData[2048] = { 0 };
        int iDecryptDataLen = 0;
        if (crypto_aes128_decrypt_base64(m_acAESKey,
            (unsigned char*)strResult.c_str(),
            strResult.length(),
            pcDecryptData, &iDecryptDataLen) != 0)
            return ;

        std::string strDecryptResult =
            std::string((char*)pcDecryptData, iDecryptDataLen);
        logPrint(MX_LOG_INFOR,"get bind state: %s",strDecryptResult.c_str());
        if(strDecryptResult.length() > 0)
        {
            std::string strCode;
            std::string strErrMsg;
            cJSON *jsonRoot = cJSON_Parse(strDecryptResult.c_str());
            if (jsonRoot)
            {
                cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
                if (jsonCode)
                {
                    strCode = std::string(jsonCode->valuestring);
                }
                else
                {
                    cJSON_Delete(jsonRoot);
                    return ;
                }
                cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "errMsg");
                if (jsonErrMsg)
                {
                    char *pcErrMsg = cJSON_Print(jsonErrMsg);
                    if (pcErrMsg)
                    {
                        strErrMsg = std::string(pcErrMsg);
                        free(pcErrMsg);
                    }

                }
                cJSON_Delete(jsonRoot);
                if(strCode.compare("200") == 0)
                {
                    m_objIPCManageBindState->setBindState(E_BIND_SUCCESS);
                    return;
                }
                else
                {
                    return ;
                }
            }
            else
            {
                return ;
            }
        }
        else
        {
            return ;
        }

        return;
    }

	std::string CIPCManageModule::sendAllIPCDevInfo(std::string strEvent, 
		std::string strParam)
	{
		std::string strResult;
		
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		
		cJSON_ReplaceItemInObject(jsonRoot, "event", cJSON_CreateString(strEvent.c_str()));

		std::string strEventConfig = cJSON_PrintUnformatted(jsonRoot);
		cJSON_Delete(jsonRoot);


        int ret = 0;
        size_t iAESParmLen = 0;
        char *acAESConfig = (char *)malloc(2 * strEventConfig.length());
        if (acAESConfig == NULL)
            return procResult(std::string("500"), "",
                "acAESConfig malloc error");

        memset(acAESConfig, 0, 2 * strEventConfig.length());
        ret = crypto_aes128_encrypt_base64(m_acAESKey,
            (unsigned char*)strEventConfig.c_str(),
            strEventConfig.length(),
            (unsigned char*)acAESConfig,
            10 * strEventConfig.length(),
            &iAESParmLen);
        if(ret < 0)
        {
            free(acAESConfig);
            return procResult(std::string("500"), "",
                "acAESConfig crypto error");
        }

        strResult = output(m_strIPCAgentGUID, m_strIPCAgentRemoteEventServer,
            (unsigned char*)acAESConfig, iAESParmLen);
        free(acAESConfig);

        unsigned char pcDecryptData[20480] = { 0 };
        int iDecryptDataLen = 0;
        if (crypto_aes128_decrypt_base64(m_acAESKey,
            (unsigned char*)strResult.c_str(),
            strResult.length(),
            pcDecryptData, &iDecryptDataLen) != 0)
            return std::string("");

        std::string strDecryptResult =
            std::string((char*)pcDecryptData, iDecryptDataLen);

        return strDecryptResult;
	}

	std::string CIPCManageModule::sendPirFalseWakeup()
	{
		char *pCount = getFWParaConfig("pir_false_wakeup_count");
		if(pCount != NULL)
		{
			int iCount = atoi(pCount);
			if(iCount >= 20)
			{
				setFWParaConfig("pir_false_wakeup_count", "0", 1);

				char* pDid = getFWParaConfig("factory", "did");
				if(pDid != NULL)
				{
					cJSON *pJsonParam = cJSON_CreateObject();
					cJSON_AddStringToObject(pJsonParam, "did", pDid);
					std::string strOut = std::string(cJSON_Print(pJsonParam));
					sendEventToIPC("PirFrequentFalseWakeup", strOut);
				}
			}
		}
		else
		{
			setFWParaConfig("pir_false_wakeup_count", "1", 1);
		}


		return procResult(std::string("200"), "", "");
	}

	std::string CIPCManageModule::sendOTAStatusToIPC(
		std::string strEvent, std::string strParam)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_Parse(strParam.c_str());

		cJSON_AddStringToObject(jsonRoot, "event", strEvent.c_str());
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
		char *out = cJSON_Print(jsonRoot);
		std::string strEventConfig = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		int ret = 0;
		size_t iAESParmLen = 0;
		char *acAESConfig = (char *)malloc(2 * strEventConfig.length());
		if (acAESConfig == NULL)
			return procResult(std::string("500"), "",
				"acAESConfig malloc error");

		memset(acAESConfig, 0, 2 * strEventConfig.length());
		ret = crypto_aes128_encrypt_base64(m_acAESKey,
			(unsigned char*)strEventConfig.c_str(),
			strEventConfig.length(),
			(unsigned char*)acAESConfig,
			2 * strEventConfig.length(),
			&iAESParmLen);
        if(ret < 0)
        {
            free(acAESConfig);
            return procResult(std::string("500"), "",
                "acAESConfig crypto error");
        }

		strResult = output(m_strIPCAgentGUID, m_strIPCAgentRemoteEventServer,
			(unsigned char*)acAESConfig, iAESParmLen);

		free(acAESConfig);

		unsigned char pcDecryptData[2048] = { 0 };
		int iDecryptDataLen = 0;
		if (crypto_aes128_decrypt_base64(m_acAESKey,
			(unsigned char*)strResult.c_str(),
			strResult.length(),
			pcDecryptData, &iDecryptDataLen) != 0)
			return std::string("");

		std::string strDecryptResult =
			std::string((char*)pcDecryptData, iDecryptDataLen);

        logPrint(MX_LOG_DEBUG, "strDecryptResult: %s", strDecryptResult.c_str());

		return strDecryptResult;
		
	}

    std::string CIPCManageModule::sendLogDataToIPC(
        std::string strEvent, std::string strParam)
    {
        std::string strResult;
        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonParam = cJSON_Parse(strParam.c_str());

        cJSON_AddStringToObject(jsonRoot, "event", strEvent.c_str());
        cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        char *out = cJSON_Print(jsonRoot);
        std::string strEventConfig = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        int ret = 0;
        size_t iAESParmLen = 0;
        char *acAESConfig = (char *)malloc(2 * strEventConfig.length());
        if (acAESConfig == NULL)
            return procResult(std::string("500"), "",
                "acAESConfig malloc error");

        memset(acAESConfig, 0, 2 * strEventConfig.length());
        ret = crypto_aes128_encrypt_base64(m_acAESKey,
            (unsigned char*)strEventConfig.c_str(),
            strEventConfig.length(),
            (unsigned char*)acAESConfig,
            2 * strEventConfig.length(),
            &iAESParmLen);
        if(ret < 0)
        {
            free(acAESConfig);
            return procResult(std::string("500"), "",
                "acAESConfig crypto error");
        }

        strResult = output(m_strIPCAgentGUID, m_strIPCAgentRemoteEventServer,
            (unsigned char*)acAESConfig, iAESParmLen);
        free(acAESConfig);

        unsigned char pcDecryptData[2048] = { 0 };
        int iDecryptDataLen = 0;
        if (crypto_aes128_decrypt_base64(m_acAESKey,
            (unsigned char*)strResult.c_str(),
            strResult.length(),
            pcDecryptData, &iDecryptDataLen) != 0)
            return std::string("");

        std::string strDecryptResult =
            std::string((char*)pcDecryptData, iDecryptDataLen);

        return strDecryptResult;
    }

	std::string CIPCManageModule::sendEventToLocalStorage(
		std::string strEvent, std::string strParam)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_Parse(strParam.c_str());

		cJSON_AddStringToObject(jsonRoot, "event", "EventOccurrence");
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strEventConfig = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return output(m_strLocalStorageGUID, 
			m_strLocalStorageRemoteEventServer,
			(unsigned char*)strEventConfig.c_str(), 
			strEventConfig.length());
	}

	std::string CIPCManageModule::sendEventToCenterSystem(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_Parse(strParam.c_str());

		cJSON_AddStringToObject(jsonRoot, "event", "eventOccurs");
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strMsg = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);


		std::string strRet = output(m_strCenterSubSystemGUID, 
			m_strCenterSubSystemServer,
			(unsigned char*)strMsg.c_str(), 
			strMsg.length());

		logPrint(MX_LOG_DEBUG, "ipc send event to center system: %s, ret:%s", strMsg.c_str(), strRet.c_str());

		return strRet;
	}
	std::string CIPCManageModule::sendSpecData(
		std::string strEvent,std::string strParam)
	{
		std::string strResult; 
		std::string strSpecData = strParam;
		int ret = 0;
		size_t iAESParmLen = 0;
		char *acAESConfig = (char *)malloc(2 * strSpecData.length());
		if (acAESConfig == NULL)
			return procResult(std::string("500"), "",
				"acAESConfig malloc error");

		memset(acAESConfig, 0, 2 * strSpecData.length());
		ret = crypto_aes128_encrypt_base64(m_acAESKey,(unsigned char*)strSpecData.c_str(),strSpecData.length(),
			(unsigned char*)acAESConfig,2 * strSpecData.length(),&iAESParmLen);

        if(ret < 0)
        {
            free(acAESConfig);
            return procResult(std::string("500"), "",
                "acAESConfig crypto error");
        }

		logPrint(MX_LOG_DEBUG, "send spec data:%s, length:%d", strSpecData.c_str(), strSpecData.length());

		strResult = output(m_strIPCAgentGUID ,m_strIPCAgentRemoteEventServer,
						(unsigned char*)acAESConfig, iAESParmLen);

		if (acAESConfig)
		{
			free(acAESConfig);
			acAESConfig =NULL;
		}
		
		logPrint(MX_LOG_DEBUG, "strResult:%s, length:%d", strResult.c_str(), strResult.length());
		unsigned char pcDecryptData[2048] = { 0 };
		int iDecryptDataLen = 0;
		if (crypto_aes128_decrypt_base64(m_acAESKey,(unsigned char*)strResult.c_str(),
			strResult.length(),pcDecryptData, &iDecryptDataLen) != 0)
			return std::string("");

		std::string strDecryptResult =
			std::string((char*)pcDecryptData, iDecryptDataLen);

		return strDecryptResult;
	}

	std::string CIPCManageModule::ipcAgentConfig(std::string strParam)
	{
		logPrint(MX_LOG_DEBUG, "ipcAgentConfig strParam: %s", strParam.c_str());
		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strModuleGUID;
		std::string strModuleName;
		std::string strServerName;
		int iType = 0;
		std::string strIP;
		int iPort = 0;
		std::string strUnix;
		int iLen = 0;
		int iPersistentPort = 0;
		if (jsonRoot)
		{
			cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
			if (jsonParam)
			{
				cJSON *jsonModuleGUID = cJSON_GetObjectItem(jsonParam, "moduleGUID");
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

				cJSON *jsonModuleName = cJSON_GetObjectItem(jsonParam, "moduleName");
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

				cJSON *jsonServerName = cJSON_GetObjectItem(jsonParam, "serverName");
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

				cJSON *jsonType = cJSON_GetObjectItem(jsonParam, "type");
				if (!jsonType)
				{
					cJSON_Delete(jsonRoot);
					return procResult(std::string("500"), "",
						std::string("type param parse failed"));
				}
				else
				{
					iType = jsonType->valueint;
					if (iType == 5)
						iType = 1;
				}

				cJSON *jsonIP = cJSON_GetObjectItem(jsonParam, "ip");
				if (!jsonIP)
				{
					cJSON_Delete(jsonRoot);
					return procResult(std::string("500"), "",
						std::string("IP param parse failed"));
				}
				else
				{
					strIP = std::string(jsonIP->valuestring);
					if (strIP.compare("0.0.0.0") == 0)
					{
						char *pValue = getFWParaConfig("BASE_STATION_IP");
						if (pValue)
						{
							strIP = std::string(pValue); 
						}
						else
						{
							strIP = std::string(BASE_STATION_IP); 
						}
					}
				}

				cJSON *jsonPort = cJSON_GetObjectItem(jsonParam, "port");
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

				cJSON *jsonUnix = cJSON_GetObjectItem(jsonParam, "unix");
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

				cJSON *jsonLen = cJSON_GetObjectItem(jsonParam, "len");
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

				cJSON *jsonPersistentPort = cJSON_GetObjectItem(jsonParam, "persistentPort");
				if (!jsonPersistentPort)
				{
					cJSON_Delete(jsonRoot);
					return procResult(std::string("500"), "",
						std::string("len param parse failed"));
				}
				else
				{
					iPersistentPort = jsonPersistentPort->valueint;
				}

				cJSON *jsonRelayIndex = cJSON_GetObjectItem(jsonParam, "relayIndex");
				if (!jsonRelayIndex)
				{
					cJSON_Delete(jsonRoot);
					return procResult(std::string("500"), "",
						std::string("relayIndex param parse failed"));
				}
				else
				{
					m_iRelayIndex = jsonRelayIndex->valueint;
				}

				cJSON *jsonIPCIndex = cJSON_GetObjectItem(jsonParam, "ipcIndex");
				if (!jsonIPCIndex)
				{
					cJSON_Delete(jsonRoot);
					return procResult(std::string("500"), "",
						std::string("ipcIndex param parse failed"));
				}
				else
				{
					m_iIPCIndex = jsonIPCIndex->valueint;
				}
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
					strModuleName.append("create module failed"));
			}

		}
		CModule *module = dynamic_cast<CModule *>(imodule.get());
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

		objClientEndPoint->init(clientConfig, E_CLIENT_EVENT);

		if (!module->regClient(strServerName, objClientEndPoint))
		{
			return procResult(std::string("500"), "",
				std::string("client reg failed"));
		}

		m_strIPCAgentGUID = strModuleGUID;
		m_strIPCAgentRemoteEventServer = strServerName;

		setUpPersistentConnect(strIP, iPersistentPort);
		sendPirFalseWakeup();
		getAllDevInfo();
		return procResult("200", "", "");
	}

	mxbool CIPCManageModule::setUpPersistentConnect(std::string strIp, int iPort)
	{
		std::string strResult;

		if(NULL == getFWParaConfig(ENV_BIND_STATUS))
		{
			sleep(3);
		}

		// linuxPopenExecCmd(strResult, "cd /usr/wifi/ ;  ./atbm_iot_cli fw_cmd \"AT+WIFI_HEART_PKT_CLOSE\"");
		linuxPopenExecCmd(strResult, "cd /usr/wifi/ ; ./atbm_iot_cli listen_itvl 10");
		linuxPopenExecCmd(strResult,  "cd /usr/wifi/ ; ./atbm_iot_cli fw_cmd \"AT+WIFI_HEART_PKT TEXT"\
                " \\${\\\"event\\\":\\\"keepalive\\\",\\\"param\\\":{\\\"did\\\":%s,\\\"time\\\":\\\"1673017080471\\\",\\\"timeout\\\":\\\"300000\\\",\\\"soc\\\":\\\"0\\\"}}\\$"\
                " PERIOD 60000 SERVER %s PORT %d\"", m_strDID.c_str(), strIp.c_str(), iPort);

		linuxPopenExecCmd(strResult, "cd /usr/wifi/ ;  ./atbm_iot_cli add_netpattern 0 %s %d TCP WAKEUP", strIp.c_str(), iPort);
		linuxPopenExecCmd(strResult, "cd /usr/wifi/ ;  ./atbm_iot_cli wkup_event TCP_PATTERN");

		return mxtrue;
	}

	mxbool CIPCManageModule::unBind()
	{
		std::string strRet;
		std::string strOut;
		std::string strCode;
		cJSON *jsonRoot = cJSON_CreateObject();
		if (jsonRoot)
		{
			cJSON_AddStringToObject(jsonRoot, "event", "ResetIPC");

			cJSON *jsonParam = cJSON_CreateObject();
			if (jsonParam)
			{
				cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

				char *out = cJSON_PrintUnformatted(jsonRoot);
				if (out)
				{
					strOut = std::string(out);
					cJSON_free(out);
					out = NULL;
				}
			}
			cJSON_Delete(jsonRoot);
		}

		if (strOut.empty())
		{
			return mxfalse;
		}

		strRet = output(m_strMcuModuleGUID,
			m_strMcuModuleRemoteServer,
			(unsigned char*)strOut.c_str(), strOut.length());
		
		if (strRet.length() > 0)
		{
			cJSON *jsonRoot = cJSON_Parse(strRet.c_str());
			if (jsonRoot)
			{
				cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
				if (jsonCode)
				{
					strCode = std::string(jsonCode->valuestring);
				}
				
				cJSON_Delete(jsonRoot);
				jsonRoot = NULL;
			}
		}
		if (strCode.compare("200") == 0)
		{
			return mxtrue;
		}

		return mxfalse;
	}

	mxbool CIPCManageModule::playAudioFile(std::string strFileId)
	{
		logPrint(MX_LOG_DEBUG, "ipc play audio:%s", strFileId.c_str());
		std::string strJson;
		cJSON *jsonRoot = cJSON_CreateObject();
		if (jsonRoot)
		{
			cJSON_AddStringToObject(jsonRoot, "event", "PlayFile");
			cJSON *jsonParam = cJSON_CreateObject();
			if (jsonParam)
			{        
				cJSON_AddStringToObject(jsonParam, "fileId", strFileId.c_str());
				cJSON_AddNumberToObject(jsonParam,"level",4);
				cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
				char *out = cJSON_Print(jsonRoot);
				strJson = std::string(out);

				cJSON_Delete(jsonRoot);
				jsonRoot = NULL;

				cJSON_free(out);
				out = NULL;
			}
		}

		if (strJson.empty())
		{
			logPrint(MX_LOG_ERROR, "ipc play file json error");
			return mxfalse;
		}

        std::string strRet = output(m_strDevModuleGUID, m_strDevModuleRemoteServer, (unsigned char *)strJson.c_str(),
								strJson.length());
        if (strRet.length() > 0)
        {
            std::string strCode;
            std::string strErrMsg;
            cJSON *jsonRoot = cJSON_Parse(strRet.c_str());

			if (jsonRoot)
			{
				cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
				if (jsonCode)
				{
					strCode = std::string(jsonCode->valuestring);
				}
			}

            if (strCode.compare("200") == 0)
            {
				logPrint(MX_LOG_DEBUG, "ipc play audio:%s success", strFileId.c_str());
                return mxtrue;
            }
		}

		return mxfalse;
	}
}

