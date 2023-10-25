#include "log_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "log_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#include "log_manage_storage.h"
#include "log_mx.h"

namespace maix {
	CLogManageModule::CLogManageModule(std::string strGUID,
		std::string strName)
		: CModule(strGUID, strName)
	{
	
	}

	CLogManageModule::~CLogManageModule()
	{
	}

	mxbool CLogManageModule::init()
	{
        logPrint(MX_LOG_ERROR,"log init start");
		if (!initConnectModule())
			return mxfalse;
        
		if (!initServer())
			return mxfalse;
        
		if (!initLowPower())
			return mxfalse;
         
        if(!initLogStorage())
            return mxfalse;
        
        if(!initMcuRemote())
            return mxfalse;
        
        if(!initLogUpload())
            return mxfalse;
        
        sendToMcuLogOnline();
        logPrint(MX_LOG_ERROR,"log init end");
        // std::thread([this]() {

        //     usleep(10 * 1000 * 1000);
        //     cJSON *jsonParam = cJSON_CreateObject();

        //     cJSON_AddStringToObject(jsonParam, "did", "667179734");

        //     char *out = cJSON_Print(jsonParam);
        //     std::string strLogData = std::string(out);
        //     cJSON_Delete(jsonParam);
        //     if (out)
        //         free(out);
        //     m_ptrLogUpload->startUploadLog(strLogData);

		// }).detach();


		return mxtrue;
	}

	mxbool CLogManageModule::unInit()
	{
		return mxbool();
	}

    mxbool CLogManageModule::initLogUpload()
    {
        m_ptrLogUpload = std::shared_ptr<CLogManageUpload>(new CLogManageUpload(this));
        return m_ptrLogUpload->init();
    }

    mxbool CLogManageModule::initLogStorage()
    {
        m_ptrLogStorage = std::shared_ptr<CLogManageStorage>(new CLogManageStorage(this));
        return m_ptrLogStorage->init();
    }

	mxbool CLogManageModule::initServer()
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
						CLogManageRemoteEventServer * objLogManageRemoteEventServer =
							new CLogManageRemoteEventServer(this);

						if (!objLogManageRemoteEventServer)
							return mxfalse;

						objLogManageRemoteEventServer->init();

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
							objLogManageRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CLogManageModule::initConnectModule()
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
						int iType = 5;
						if (!module->getConfig(acComServerConfig, "TYPE", iType))
						{
							return mxfalse;
						}
                        if(iType == 5)
                        {
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

	mxbool CLogManageModule::initLowPower()
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

    mxbool CLogManageModule::initMcuRemote()
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

	void CLogManageModule::lowPowerRun()
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

	std::string CLogManageModule::lowPowerProc()
	{
        logPrint(MX_LOG_ERROR,"log manage enter lowpower");

        if(m_ptrLogStorage && (m_ptrLogStorage.use_count() != 0))
        {
            m_ptrLogStorage->unInit();
        }
        logPrint(MX_LOG_ERROR,"log manage uninit end");
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "LogManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

	std::string CLogManageModule::remoteEventServerProc(std::string strEvent, 
		std::string strParam)
	{
        if (0 == strEvent.compare("EnterLowPower"))
		{
			return enterLowPower(strParam);
		}
        else if (0 == strEvent.compare("StartUploadLog"))
        {
            if(m_ptrLogStorage && (m_ptrLogStorage.use_count() != 0))
            {
                m_ptrLogStorage->flushLogCache();
            }
            
            if(m_ptrLogUpload && (m_ptrLogUpload.use_count() != 0))
            {
                return m_ptrLogUpload->startUploadLog(strParam);
            }
            return procResult(std::string("500"), "",
                strEvent.append("start err"));
        }
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	std::string CLogManageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		return procResult(std::string("200"), "",
			"log manage enter low power");
	}

    void CLogManageModule::sendToMcuLogOnline()
    {
        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonParam = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "LogManageOnline");
        cJSON_AddBoolToObject(jsonParam,"onlineState",mxtrue);
        cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        char *out = cJSON_Print(jsonRoot);
        std::string strResult = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);           
        output(m_strMcuManageGUID,m_strMcuManageServer,(unsigned char*) strResult.c_str(),strResult.length());
    }

	std::string CLogManageModule::procResult(std::string code, 
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