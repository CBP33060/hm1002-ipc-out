#include "dev_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "dev_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"

namespace maix {
	CDevManageModule::CDevManageModule(
		std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
	{
	}

	CDevManageModule::~CDevManageModule()
	{
	}

	mxbool CDevManageModule::init()
	{
		if (!initConnectModule())
		{
			std::cout << "[DevManageApp]: init connect module failed" << std::endl;
			return mxfalse;
		}
			
		if (!initVideoSourceRemoteEventServer())
		{
			std::cout << "[DevManageApp]: init video source remote event server failed" << std::endl;
			return mxfalse;
		}
			

		if(!initMcuSerialPortRemoteEventServer())
		{
			std::cout << "[DevManageApp]: init mcu serial port remote event server failed" << std::endl;
			return mxfalse;
		}
			
		if(!initAiRemoteEventServer())
		{
			std::cout << "[DevManageApp]: init ai remote event server failed" << std::endl;
			return mxfalse;
		}
			

		if (!initServer())
		{
			std::cout << "[DevManageApp]: init server failed" << std::endl;
			return mxfalse;
		}

		if (!initPalyVoice())
		{
			std::cout << "[DevManageApp]: init play voice failed" << std::endl;
			return mxfalse;
		}
		
		if (!initLowPower())
		{
			std::cout << "[DevManageApp]: init low power failed" << std::endl;
			return mxfalse;
		}
			
		if (!initMcuSerialPortRemoteEventServer())
		{
			std::cout << "[DevManageApp]: init mcu serial port remote event server failed" << std::endl;
			return mxfalse;
		}
		
		if (!initAiRemoteEventServer())
		{
			std::cout << "[DevManageApp]: init ai remote event server failed" << std::endl;
			return mxfalse;
		}
			
		m_objDevSpecHandle = std::shared_ptr<CDevSpecHandle>(
			new CDevSpecHandle(this));
		if(NULL == m_objDevSpecHandle)
		{
			return mxfalse;
		}
		if (!m_objDevSpecHandle->init())
		{
			std::cout << "[DevManageApp]: init spec handle failed" << std::endl;
			return mxfalse;
		}
		
		return mxtrue;
	}

	mxbool CDevManageModule::unInit()
	{
		return mxbool();
	}

	mxbool CDevManageModule::initServer()
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
						CDevManageRemoteEventServer * objDevManageRemoteEventServer =
							new CDevManageRemoteEventServer(this);

						if (!objDevManageRemoteEventServer)
							return mxfalse;

						objDevManageRemoteEventServer->init();

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
							objDevManageRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool CDevManageModule::initConnectModule()
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
				}
			}

		}

		return mxtrue;
	}

	mxbool CDevManageModule::initLowPower()
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

	void CDevManageModule::lowPowerRun()
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

	std::string CDevManageModule::lowPowerProc()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "DevManageExit");
		cJSON_AddNumberToObject(jsonParam, "allow", 200);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_PrintUnformatted(jsonRoot);
		std::string strResult = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strResult;
	}

	mxbool CDevManageModule::initPalyVoice()
	{
		m_objDevPlayVoice = std::shared_ptr<CDevPlayVoice>(
			new CDevPlayVoice(this));

		if (!m_objDevPlayVoice->init())
			return mxfalse;
		return mxtrue;
	}

	mxbool CDevManageModule::initVideoSourceRemoteEventServer()
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

	mxbool CDevManageModule::initMcuSerialPortRemoteEventServer()
	{
		if (!getConfig("MCU_SERIAL_REMOTE_EVENT", "GUID", m_strMcuSerialPortGUID))
		{
			return mxfalse;
		}

		if (!getConfig("MCU_SERIAL_REMOTE_EVENT", "SERVER", m_strMcuSerialPortServer))
		{
			return mxfalse;
		}
		return mxtrue;
	}

	mxbool CDevManageModule::initAiRemoteEventServer()
	{
		if (!getConfig("AI_REMOTE_EVENT", "GUID", m_strAiGUID))
		{
			return mxfalse;
		}

		if (!getConfig("AI_REMOTE_EVENT", "SERVER", m_strAiServer))
		{
			return mxfalse;
		}
		return mxtrue;
	}

	std::string CDevManageModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		if (0 == strEvent.compare("EnterLowPower"))
		{
			return enterLowPower(strParam);
		}
        else if (0 == strEvent.compare("PlayFile"))
        {
            return playWithFileId(strParam);
        }
        else if (0 == strEvent.compare("PlayPathFile"))
        {
            return playWithFilePath(strParam);
        }
        else if (0 == strEvent.compare("PlayFileStop"))
        {
            return stopWithFileId(strParam);
        }
		else if (0 == strEvent.compare("devConfig"))
		{
			return handleDevConfig(strParam);
		}else if (0 == strEvent.compare("AiEvent"))
		{
			return getAiEvent(strParam);
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	std::string CDevManageModule::procResult(std::string code,
		std::string strMsg, std::string strErr)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonMsg = cJSON_Parse(strMsg.c_str());
		cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
		cJSON_AddItemToObject(jsonRoot, "msg", jsonMsg);
		cJSON_AddStringToObject(jsonRoot, "errMsg", strErr.c_str());
		char *pcResult = cJSON_PrintUnformatted(jsonRoot);
		strResult = std::string(pcResult);
		cJSON_Delete(jsonRoot);
		if (pcResult)
			free(pcResult);

		return strResult;
	}

	std::string CDevManageModule::getVideoModuleGuid()
	{
		return m_strVideoModuleGUID;
	}

	std::string CDevManageModule::getVideoModuleRmoteServerName()
	{
		return m_strVideoModuleRemoteServer;
	}

	std::string CDevManageModule::getMcuSerialPortGUID()
	{
		return m_strMcuSerialPortGUID;
	}
	std::string CDevManageModule::getMcuSerialPortServer()
	{
		return m_strMcuSerialPortServer;
	}

	std::string CDevManageModule::getLowPowerGUID()
	{
		return m_strLowPowerGUID;
	}
	std::string CDevManageModule::getLowPowerServer()
	{
		return m_strLowPowerServer;
	}

	std::string CDevManageModule::getAiGUID()
	{
		return m_strAiGUID;
	}
	std::string CDevManageModule::getAiServer()
	{
		return m_strAiServer;
	}

	std::string CDevManageModule::outputMsg(std::string strGUID, std::string strModuleName,
											std::string strEvent, std::string strValue)
	{
        cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "devConfig");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonParam, "configName", strEvent.c_str());

		cJSON *jsonConfigParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonConfigParam, "value", strValue.c_str());

		cJSON_AddItemToObject(pJsonParam, "configParam", jsonConfigParam);
		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);

        char *out = cJSON_PrintUnformatted(pJsonRoot);

		std::string strRet = output(strGUID, strModuleName, (unsigned char *)out, strlen(out) + 1);

        if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }

		return strRet;
	}

	std::string CDevManageModule::outputConfigVideo(std::string strGUID, std::string strModuleName,
											std::string strEvent, std::string strValue)
	{
        cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "configVideo");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonParam, "interfaceName", "CLoadZeratulVideoInterface");
		cJSON_AddStringToObject(pJsonParam, "configName", strEvent.c_str());

		cJSON *jsonValue = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonValue, "value", strValue.c_str());

		cJSON_AddItemToObject(pJsonParam, "configValue", jsonValue);
		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);

        char *out = cJSON_PrintUnformatted(pJsonRoot);

		std::string strRet = output(strGUID, strModuleName, (unsigned char *)out, strlen(out) + 1);

        if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }

		return strRet;
	}
	
	std::string CDevManageModule::enterLowPower(std::string strParam)
	{
		{
			std::unique_lock<std::mutex> lock(m_mutexLowPower);
			m_conditionLowPower.notify_one();
		}
		return procResult(std::string("200"), "",
			"dev manage enter low power");
	}

    std::string CDevManageModule::playWithFileId(std::string strParam)
    {
        std::string fileId;
        int level = 0;
        int playTime = 0;
        std::string errCode;
        std::string errMsg;
        cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
        if(pJsonRoot)
        {
            cJSON *pFileId = cJSON_GetObjectItem(pJsonRoot,"fileId");
            if(pFileId)
            {
                errCode = "200";
                errMsg = "success";
                fileId = pFileId->valuestring;
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
            cJSON *pLevel = cJSON_GetObjectItem(pJsonRoot,"level");
            if(pLevel)
            {
                errCode = "200";
                errMsg = "success";
                level = pLevel->valueint;
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
            cJSON *pPlayTime = cJSON_GetObjectItem(pJsonRoot,"playTime");
            if(pPlayTime)
            {
                playTime = pPlayTime->valueint;
            }
            cJSON_Delete(pJsonRoot);
        }
        else
        {
            errCode = "404";
            errMsg = "Params err";
        }
        if(errCode.compare("200") == 0)
        {
            mxbool result = m_objDevPlayVoice->playWithFileId(fileId,level,playTime);
            if(result)
            {
                errCode = "200";
                errMsg = "success";
            }
            else
            {
                errCode = "500";
                errMsg = "module processing error";
            }
        }
        return procResult(errCode, errMsg,
            "dev manage play file id");
    }

    std::string CDevManageModule::playWithFilePath(std::string strParam)
    {
        std::string filePath;
        int level = 0;
        int playTime = 0;
        std::string errCode;
        std::string errMsg;
        cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
        if(pJsonRoot)
        {
            cJSON *pFileId = cJSON_GetObjectItem(pJsonRoot,"fileId");
            if(pFileId)
            {
                errCode = "200";
                errMsg = "success";
                filePath = pFileId->valuestring;
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
            cJSON *pLevel = cJSON_GetObjectItem(pJsonRoot,"level");
            if(pLevel)
            {
                errCode = "200";
                errMsg = "success";
                level = pLevel->valueint;
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
            cJSON *pPlayTime = cJSON_GetObjectItem(pJsonRoot,"playTime");
            if(pPlayTime)
            {
                playTime = pPlayTime->valueint;
            }
            cJSON_Delete(pJsonRoot);
        }
        else
        {
            errCode = "404";
            errMsg = "Params err";
        }
        if(errCode.compare("200") == 0)
        {
            mxbool result = m_objDevPlayVoice->playWithFilePath(filePath,level,playTime);
            if(result)
            {
                errCode = "200";
                errMsg = "success";
            }
            else
            {
                errCode = "500";
                errMsg = "module processing error";
            }
        }
        return procResult(errCode, errMsg,
            "dev manage play file path success");
    }

    std::string CDevManageModule::stopWithFileId(std::string strParam)
    {
        std::string fileId;
        std::string errCode;
        std::string errMsg;
        cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
        if(pJsonRoot)
        {
            cJSON *pFileId = cJSON_GetObjectItem(pJsonRoot,"fileId");
            if(pFileId)
            {
                errCode = "200";
                errMsg = "success";
                fileId = pFileId->valuestring;
            }
            else
            {
                errCode = "404";
                errMsg = "Params err";
            }
            cJSON_Delete(pJsonRoot);
        }
        else
        {
            errCode = "404";
            errMsg = "Params err";
        }
        if(errCode.compare("200") == 0)
        {
            mxbool result = mxfalse;
            if(m_objDevPlayVoice)
            {
                result = m_objDevPlayVoice->stopWithFileId(fileId);
            }
            if(result)
            {
                errCode = "200";
                errMsg = "success";
            }
            else
            {
                errCode = "500";
                errMsg = "module processing error";
            }
        }
        return procResult(errCode, errMsg,
            "mike manage stop file id");
    }


	std::string CDevManageModule::handleDevConfig(std::string strParam)
	{
		logPrint(MX_LOG_DEBUG, "dev event proc strParam:%s", strParam.c_str());


		return m_objDevSpecHandle->handleSpec(strParam);

	}

	std::string CDevManageModule::getAiEvent(std::string strParam)
	{
		std::string resultMsg;
		std::string strObjValue;
		int iValue;

		char *AutoAlarm_status = getFWParaConfig("user", "auto_audible_visual_alarm");
		if(AutoAlarm_status != NULL)
		{
			// printf("AutoAlarm_status :%d\n", atoi(AutoAlarm_status));
            logPrint(MX_LOG_DEBUG, "AutoAlarm_status : %d\n", atoi(AutoAlarm_status));
			if(atoi(AutoAlarm_status) == 0)
			{
				resultMsg = procResult(std::string("500"), "", "device is not AutoAlarm status");
				return resultMsg;
			}

			if(!access("/tmp/_liveing", F_OK))
			{
				
				resultMsg = procResult(std::string("300"), "", "device is _liveing");
				return resultMsg;
			}
			
			if(!access("/tmp/_ai_event", F_OK))
			{
				resultMsg = procResult(std::string("300"), "", "device is AutoAlarming");
				return resultMsg;
			}

			system("touch /tmp/_ai_event");

			iValue = 5;
			strObjValue = std::to_string(iValue); //打开白光灯
			outputConfigVideo(m_strVideoModuleGUID, m_strVideoModuleRemoteServer, EVENT_SET_NIGHT_SHOT, strObjValue);

			cJSON *jsonParam = cJSON_CreateObject(); //打开喇叭报警
			cJSON_AddStringToObject(jsonParam, "fileId", "warming_alarm");
			cJSON_AddNumberToObject(jsonParam,"level", 1);
			cJSON_AddNumberToObject(jsonParam,"playTime",30);
			char *out = cJSON_Print(jsonParam);
			std::string strJsonParam = std::string(out);
			cJSON_Delete(jsonParam);
			if (out)
			{
				free(out);
				out = NULL;
			}

			playWithFileId(strJsonParam);

		}
		else
		{
			logPrint(MX_LOG_ERROR, "getFWParaConfig get AutoAlarm_status failed\n");
			resultMsg = procResult(std::string("400"), "", "getFWParaConfig get AutoAlarm_status failed");
			return resultMsg;
		}

		return procResult(std::string("200"), "","");
	}

}
