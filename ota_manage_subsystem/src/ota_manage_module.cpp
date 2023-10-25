#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include "ota_manage_module.h"
#include "com_rpc_server_end_point.h"
#include "ota_manage_remote_event_server.h"
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "log_mx.h"
#include "mbedtls/base64.h"
#include "ota_manage_fw_unpack.h"
#include "common.h"
#include "fw_env_para.h"


#define FW_BIN_PATH  "/userfs/ota.bin"

#define FW_VERSION_PATH_TMP "/etc/version_tmp"
#define FW_VERSION_PATH  "/etc/version"
#define FW_VERSION_NAME  "version"

#define FW_A_PARTITION	0
#define FW_B_PARTITION	1

#define FW_KERNEL_PARTITION 			"/dev/mtd2"
#define FW_KERNEL_BACK_PARTITION 		"/dev/mtd12"
// #define FW_KERNEL_BACK_PARTITION "/userfs/kernel"

#define FW_ROOTFS_PARTITION 			"/dev/mtd3"
#define FW_ROOTFS_BACK_PARTITION 		"/dev/mtd13"

#define FW_SYSTEM_A_PARTITION 			"/dev/mtd8"
#define FW_SYSTEM_B_PARTITION 			"/dev/mtd11"
#define FW_MODEL_A_PARTITION  			"/dev/mtd7"
#define FW_MODEL_B_PARTITION  			"/dev/mtd10"
#define FW_MCU_PARTITION  	  			"/dev/mtd6"


namespace maix {
	void OTATimeOutProc(COTAManageModule* module)
	{
		logPrint(MX_LOG_INFOR, "OTATimeOutProc");
		if (module)
		{
			module->exitOTAProc();
		}
	}

	COTAManageModule::COTAManageModule(std::string strGUID, std::string strName)
		: CModule(strGUID, strName)
		, m_pFD(NULL)
		, m_iFileLen(0)
		, m_iFileOffset(0)
		, m_iIndex(0)
		, m_iTimeout(0)
		, m_bStart(mxfalse)
		, m_bLoadConfig(mxfalse)
		, m_eOTAStage(E_OTA_STA_IDLE)
	{
	}

	COTAManageModule::~COTAManageModule()
	{
	}

	mxbool COTAManageModule::init()
	{
		char *pDid = getFWParaConfig("factory", "did");
		if(!pDid)
		{
			logPrint(MX_LOG_ERROR, "Failed to get did");
			return mxfalse;
		}
		m_strDID = std::string(pDid);

		if (!initConnectModule())
			return mxfalse;

		if (!initOtaManageRemoteEvent())
			return mxfalse;

		if (!initServer())
			return mxfalse;

		if (!initUpgradeProcess())
			return mxfalse;

		if (!initProgressReport())
			return mxfalse;

		if (!initUpgradeStatus())
			return mxfalse;

		if (!initAIRemoteEvent())
			return mxfalse;

		if (!initEventManageRemoteEvent())
			return mxfalse;

		return mxtrue;
	}

	mxbool COTAManageModule::unInit()
	{
		return mxbool();
	}

	mxbool COTAManageModule::initServer()
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
						COTAManageRemoteEventServer * objOTAManageRemoteEventServer =
							new COTAManageRemoteEventServer(this);

						if (!objOTAManageRemoteEventServer)
							return mxfalse;

						objOTAManageRemoteEventServer->init();

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
							objOTAManageRemoteEventServer);

						if (!regServer(strName, objServerEndPoint))
							return mxfalse;
					}
				}
			}
		}

		return mxtrue;
	}

	mxbool COTAManageModule::initConnectModule()
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
						else
						{
							if (iType == 5)
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

	mxbool COTAManageModule::initUpgradeProcess()
	{
		COTAManageUpgradeProcess *objOTAManageUpgradeProcess =
			new COTAManageUpgradeProcess(this);

		if (objOTAManageUpgradeProcess == NULL)
			return mxfalse;

		if (!objOTAManageUpgradeProcess->init())
			return mxfalse;

		m_threadOTAManageUpgradeProcess = 
			std::thread([objOTAManageUpgradeProcess]() {
			objOTAManageUpgradeProcess->run();
		});

		m_objOTAManageUpgradeProcess = objOTAManageUpgradeProcess;
		return mxtrue;
	}

	mxbool COTAManageModule::initOtaManageRemoteEvent()
	{
        if (!getConfig("DEV_REMOTE_SERVER", "GUID", m_strDevManageGUID))
		{
			return mxfalse;
		}
		
		if (!getConfig("DEV_REMOTE_SERVER", "SERVER", m_strDevManageServer))
		{
			return mxfalse;
		}

        if (!getConfig("MCU_REMOTE_EVENT", "GUID", m_strMcuManageGUID))
        {
            return mxfalse;
        }
        
        if (!getConfig("MCU_REMOTE_EVENT", "SERVER", m_strMcuManageServer))
        {
            return mxfalse;
        }

		return mxtrue;
	}

	mxbool COTAManageModule::initAIRemoteEvent()
	{
		if (!getConfig("AI_REMOTE_EVENT", "GUID", m_strAIGUID))
		{
			return mxfalse;
		}

		if (!getConfig("AI_REMOTE_EVENT", "SERVER", m_strAIServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool COTAManageModule::initEventManageRemoteEvent()
	{
        if (!getConfig("EVENT_MANAGE_REMOTE_EVENT", "GUID", m_strEventManageGUID))
		{
			return mxfalse;
		}
		
		if (!getConfig("EVENT_MANAGE_REMOTE_EVENT", "SERVER", m_strEventManageServer))
		{
			return mxfalse;
		}

		logPrint(MX_LOG_DEBUG, "init event manage event server:%s, guid:%s", m_strEventManageServer.c_str(), m_strEventManageGUID.c_str());

		return mxtrue;
	}

	mxbool COTAManageModule::initProgressReport()
	{
		COTAManageProgressReport *objOTAManageProgressReport =
			new COTAManageProgressReport(this);

		if (objOTAManageProgressReport == NULL)
			return mxfalse;

		if (!objOTAManageProgressReport->init())
			return mxfalse;

		m_threadOTAManageProgressReport =
			std::thread([objOTAManageProgressReport]() {
			objOTAManageProgressReport->run();
		});

		m_objOTAManageProgressReport = objOTAManageProgressReport;

		return mxtrue;
	}

	mxbool COTAManageModule::initUpgradeStatus()
	{
		std::string strValue;
		strValue.clear();
		linuxPopenExecCmd(strValue, "tag_env_info --set OTA ota_step 0");
		strValue.clear();
		linuxPopenExecCmd(strValue, "fw_printenv user ota_sta");

		if (!strValue.empty() && m_objOTAManageUpgradeProcess)
		{
			int iValue = atoi(strValue.c_str());
			if (m_objOTAManageUpgradeProcess)
			{
				if ((E_OTA_UPGRADE_IDLE != iValue) && (E_OTA_UPGRADE_COMPLETE != iValue))
				{
					m_objOTAManageUpgradeProcess->setOTAStage(E_OTA_UPGRADE_FAILED);
				}
				else if (E_OTA_UPGRADE_COMPLETE == iValue)
				{
					if (access(FW_BIN_PATH, 0) == 0)
					{
						remove(FW_BIN_PATH);
					}
					if (m_objOTAManageProgressReport)
					{
						T_OTAReportData data;
						data.eReportType = E_R_PROGRESS;
						data.lTimeStamp = getCurrentTime() + 10 * 1000;
						data.iTryNum = 0;
						data.strData = reportProcessData("unpack", "200", 100);

						m_objOTAManageProgressReport->pushData(data);
					}
					playAudioFile(FILE_OTA_SUCCESS);
					setLedStatus(LedInfo_Color_BLUE, LedInfo_State_ON, 200, 0);
					strValue.clear();
					linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_IDLE);
					m_objOTAManageUpgradeProcess->setOTAStage(E_OTA_UPGRADE_IDLE);
					strValue.clear();
					linuxPopenExecCmd(strValue, "fw_setenv user %s %d", LOWPOWER_MODE, ENABLE_LOWPOWER);
				}
			}
			logPrint(MX_LOG_INFOR, "init upgrade status:%d", iValue);
		}
		strValue.clear();
		linuxPopenExecCmd(strValue, "tag_env_info --get HW 70mai_system_partition");
		int iPos = strValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_system_partition");
			return mxfalse;
		}

		strValue = strValue.substr(iPos + strlen("Value="), strValue.length() - strlen("Value="));
		m_iSystemPartition = atoi(strValue.c_str());

		logPrint(MX_LOG_INFOR, "init upgrade partition:%d", m_iSystemPartition);

		return mxtrue;
	}

	std::string COTAManageModule::remoteEventServerProc(
		std::string strEvent, std::string strParam)
	{
		if (0 == strEvent.compare("StartOTA"))
		{
			return startOTA(strParam);
		}
		else if (0 == strEvent.compare("OTAData"))
		{
			return OTAData(strParam);
		}
		else if (0 == strEvent.compare("SendOTAEnd"))
		{
			return sendOTAEnd(strParam);
		}
		else if (0 == strEvent.compare("DirectStartOTA"))
		{
			return directStartOTA(strParam);
		}
		else
		{
			return procResult(std::string("400"), "",
				strEvent.append("  event not support"));
		}
	}

	std::string COTAManageModule::procResult(std::string code, 
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

	mxbool COTAManageModule::sendAlertEvent(const std::string &strValue)
	{
		logPrint(MX_LOG_DEBUG, "ota send alert event");

		std::string strJson;
		std::string strCode;
		cJSON *jsonRoot = cJSON_CreateObject();
		if (jsonRoot)
		{
			cJSON_AddStringToObject(jsonRoot, "event", "AlertEvent");
			cJSON *jsonParam = cJSON_CreateObject();
			if (jsonParam)
			{        
				cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
				cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());
				cJSON_AddStringToObject(jsonParam, "value", strValue.c_str());

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
			logPrint(MX_LOG_ERROR, "ota send alert event error");
			return mxfalse;
		}

        std::string strRet = output(m_strEventManageGUID, m_strEventManageServer, (unsigned char *)strJson.c_str(),
								strJson.length());
		logPrint(MX_LOG_INFOR, "ota send alert event msg:%s, ret:%s", strJson.c_str(), strRet.c_str());
        if (strRet.length() > 0)
        {
            jsonRoot = cJSON_Parse(strRet.c_str());
			
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
			logPrint(MX_LOG_INFOR, "ota send alert event sucess");
			return mxtrue;
		}

		return mxfalse;
	}

	mxbool COTAManageModule::checkBatteryLevel()
	{
		logPrint(MX_LOG_INFOR, "ota check battery level");
		int iValue = 0;
		std::string strJson;
		cJSON *jsonRoot = cJSON_CreateObject();
		if (jsonRoot)
		{
			cJSON_AddStringToObject(jsonRoot, "event", "GetBatteryLevel");
			cJSON *jsonParam = cJSON_CreateObject();
			if (jsonParam)
			{        
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
			logPrint(MX_LOG_ERROR, "ota check battery error");
			return mxfalse;
		}

        std::string strRet = output(m_strMcuManageGUID, m_strMcuManageServer, (unsigned char *)strJson.c_str(),
								strJson.length());
		logPrint(MX_LOG_INFOR, "ota check battery msg:%s, ret:%s", strJson.c_str(), strRet.c_str());
        if (strRet.length() > 0)
        {
            jsonRoot = cJSON_Parse(strRet.c_str());
			
			if (jsonRoot)
			{
				cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
				if (jsonParam)
				{
					cJSON *jsonValue = cJSON_GetObjectItem(jsonParam, "capacityValue");
					if(jsonValue)
					{
						iValue = jsonValue->valueint;
					}
				}

				cJSON_Delete(jsonRoot);
				jsonRoot = NULL;
				if (iValue <= OTA_LOW_BATTERY)
				{
					sendAlertEvent(std::to_string(E_EVENT_LOW_POWER_OTA));
					logPrint(MX_LOG_INFOR, "ota check battery level failed, value:%d", iValue);
					return mxfalse;
				}
				else
				{
					logPrint(MX_LOG_INFOR, "ota check battery level successfully");
					return mxtrue;
				}
			}
		}

		logPrint(MX_LOG_INFOR, "ota check battery level failedxx");
		return mxfalse;
	}

	std::string COTAManageModule::startOTA(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "startOTA");

		std::string strIdleResult;
		if (!isDevIdle(strIdleResult))
		{
			return strIdleResult;
		}

		m_iFileLen = 0;
		m_iFileOffset = 0;
		m_iIndex = 0;

		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		std::string strDID;

		if (jsonRoot)
		{
			cJSON *jsonDID = cJSON_GetObjectItem(jsonRoot, "did");
			if (!jsonDID)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("did param parse failed"));
			}
			else
			{
				strDID = std::string(jsonDID->valuestring);
			}

			cJSON *jsonFileLen = cJSON_GetObjectItem(jsonRoot, "fileLen");
			if (!jsonFileLen)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("fileLen param parse failed"));
			}
			else
			{
				m_iFileLen = jsonFileLen->valueint;
			}

			cJSON *jsonTimeout = cJSON_GetObjectItem(jsonRoot, "timeout");
			if (!jsonTimeout)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("fileLen param parse failed"));
			}
			else
			{
				m_iTimeout = jsonTimeout->valueint;
			}

			cJSON_Delete(jsonRoot);
		}

		if (!checkBatteryLevel())
		{
			return procResult(std::string("500"), "",
				std::string("battert level low level"));
		}
		
		if (m_iTimeout > 0)
		{
			m_timerOTATimeout.StartTimerOnce(m_iTimeout * 1000,
				std::bind(OTATimeOutProc, this));
		}		
		m_bStart = mxtrue;

		if (access(FW_BIN_PATH, 0) == 0)
		{
			remove(FW_BIN_PATH);
		}

		m_pFD = fopen(FW_BIN_PATH, "wb+");

		if (m_pFD == NULL)
		{
			return procResult(std::string("502"), "", "open file failed");
		}

		setOTAStage(E_OTA_STA_DOWNLOADING);
		playAudioFile(FILE_OTA_START);
		setLedStatus(LedInfo_Color_BLUE, LedInfo_State_FLASHING, 1, 4);

		closeAIDetect();

		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user %s %d", LOWPOWER_MODE, DISABLE_LOWPOWER);

		logPrint(MX_LOG_INFOR, "ota package start download..., timeout:%d",m_iTimeout);

		return procResult(std::string("200"), "", "OK");
	}

	std::string COTAManageModule::OTAData(std::string strParam)
	{
		if (m_pFD == NULL)
		{
			return procResult(std::string("503"), "", "file is not open");
		}

		int iDataLen = 0;
		int iIndex = 0;
		std::string strData;

		cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
		if (jsonRoot)
		{
			cJSON *jsonData = cJSON_GetObjectItem(jsonRoot, "data");
			if (!jsonData)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("data param parse failed"));
			}
			else
			{
				strData = std::string(jsonData->valuestring);
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
				iDataLen  = jsonLen->valueint;
			}

			cJSON *jsonIndex = cJSON_GetObjectItem(jsonRoot, "index");
			if (!jsonIndex)
			{
				cJSON_Delete(jsonRoot);
				return procResult(std::string("500"), "",
					std::string("index param parse failed"));
			}
			else
			{
				iIndex = jsonIndex->valueint;
			}

			cJSON_Delete(jsonRoot);
		}

		if (m_iIndex != iIndex)
		{
			return lostDataResult();
		}

		size_t iDecodeOutLen = 0;
		unsigned char ucDecodeData[strData.length() * 2] = { 0 };
		mbedtls_base64_decode(ucDecodeData,
			sizeof(ucDecodeData),
			&iDecodeOutLen,
			(unsigned char*)strData.c_str(), 
			strData.length());

		if (iDecodeOutLen != static_cast<size_t>(iDataLen))
		{
			return procResult(std::string("506"), "", "file len is error");
		}

		int iRet = fwrite(ucDecodeData, sizeof(char), iDecodeOutLen, m_pFD);
		if (static_cast<size_t>(iRet) != iDecodeOutLen)
		{
			return procResult(std::string("507"), "", "file write error");
		}
		else
		{
			m_iIndex++;
			m_iFileOffset += iRet;
		}

		if ((m_iIndex % 3) == 0)
		{
			logPrint(MX_LOG_INFOR, "ota package downloading..., len:%d, offset:%d", iDecodeOutLen, m_iFileOffset);
		}

		return procResult(std::string("200"), "", "OK");
	}

	std::string COTAManageModule::sendOTAEnd(std::string strParam)
	{
		if (m_pFD)
		{
			fclose(m_pFD);
			m_pFD = NULL;
		}

		logPrint(MX_LOG_INFOR, "ota file receive complete: %d", m_iFileOffset);

		setOTAStage(E_OTA_STA_INSTALLING);
		if (m_objOTAManageUpgradeProcess)
		{
			T_E_OTA_UPGRADE eOTAUpgradeState =
				m_objOTAManageUpgradeProcess->getOTAState();
			if (eOTAUpgradeState == E_OTA_UPGRADE_IDLE)
			{
				m_objOTAManageUpgradeProcess->setOTAStage(E_OTA_UPGRADE_CHECK);
			}
			else
			{
				procResult(std::string("400"), "", "Upgrading");
			}
		}

		return procResult(std::string("200"), "", "ok");
	}

	std::string COTAManageModule::directStartOTA(std::string strParam)
	{
		logPrint(MX_LOG_INFOR, "ota direct start");
		
		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user %s %d", LOWPOWER_MODE, DISABLE_LOWPOWER);

		setOTAStage(E_OTA_STA_INSTALLING);
		if (m_objOTAManageUpgradeProcess)
		{
			T_E_OTA_UPGRADE eOTAUpgradeState =
				m_objOTAManageUpgradeProcess->getOTAState();
			if (eOTAUpgradeState == E_OTA_UPGRADE_IDLE)
			{
				m_objOTAManageUpgradeProcess->setOTAStage(E_OTA_UPGRADE_CHECK);
			}
			else
			{
				procResult(std::string("400"), "", "Upgrading");
			}
		}

		playAudioFile(FILE_OTA_START);
		setLedStatus(LedInfo_Color_BLUE, LedInfo_State_FLASHING, 1, 4);

		return procResult(std::string("200"), "", "OK");
	}

	std::string COTAManageModule::lostDataResult()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "LostIndex");

		cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());
		cJSON_AddNumberToObject(jsonParam, "index", m_iIndex);
		cJSON_AddNumberToObject(jsonParam, "offset", m_iFileOffset);

		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strOTALostData = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return  procResult(std::string("505"), 
			strOTALostData, "ota lost data return");
	}

	mxbool COTAManageModule::isInOTAProc()
	{
		return m_bStart;
	}

	void COTAManageModule::exitOTAProc()
	{
		if (m_eOTAStage != E_OTA_STA_COMPLETE)
		{
			reportOTATimeout();
		}
		
		m_objOTAManageUpgradeProcess->setOTAStage(E_OTA_UPGRADE_FAILED);

		m_bStart = mxfalse;
	}

	mxbool COTAManageModule::isDevIdle(std::string & strResult)
	{
		T_E_OTA_STAGE eOTAState = getOTAStage();

		if (eOTAState == E_OTA_STA_IDLE ||
			eOTAState == E_OTA_STA_COMPLETE)
		{
			if (m_pFD != NULL)
			{
				fclose(m_pFD);
				m_pFD = NULL;
			}		
		}
		else
		{
			logPrint(MX_LOG_INFOR, "sendOTAEnd eOTAState: %d", eOTAState);
			strResult = procResult(std::string("501"), "", "is in ota");
			return mxfalse;
		}

		return mxtrue;
	}

	T_E_OTA_STAGE COTAManageModule::getOTAStage()
	{
		return m_eOTAStage;
	}

	mxbool COTAManageModule::setOTAStage(T_E_OTA_STAGE eStage)
	{
		m_eOTAStage = eStage;
		return mxtrue;
	}

	mxbool COTAManageModule::loadVersionConfig()
	{
		if (m_bLoadConfig)
		{
			logPrint(MX_LOG_ERROR, "version have loaded");
			return mxtrue;
		}

		const char *srcFileName = FW_VERSION_NAME;
		const char *dstFileName = FW_VERSION_PATH_TMP;
		if ((access(FW_BIN_PATH, 0) != 0) && (access(FW_VERSION_PATH, 0)) != 0)
		{
			logPrint(MX_LOG_ERROR, "access version path:%s or %s failed", srcFileName, dstFileName);
			return mxfalse;
		}
		if (!unpackFirmwareBin(FW_BIN_PATH, srcFileName, dstFileName, 0, mxfalse))
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin failed, name :%s", srcFileName);
			return mxfalse;
		}

#ifdef _INI_CONFIG

		try
		{
			INI::CINIFile iniVersionConfig;
			INI::CINIFile iniOtaPackVersionConfig;

			iniVersionConfig.load(FW_VERSION_PATH);
			iniOtaPackVersionConfig.load(FW_VERSION_PATH_TMP);
			m_strOTAPackageVersion = 
				iniVersionConfig["APP_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTAKernelVersion = 
				iniVersionConfig["KERNEL_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTARootfsVersion = 
				iniVersionConfig["ROOTFS_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTASystemVersion = 
				iniVersionConfig["SYSTEM_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTAModelVersion = 
				iniVersionConfig["MODEL_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTAMCUVersion = 
				iniVersionConfig["MCU_FW_CONFIG"]["VERSION"].as<std::string>();

			m_strOTAPackageNewVersion = 
				iniOtaPackVersionConfig["APP_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTAKernelNewVersion = 
				iniOtaPackVersionConfig["KERNEL_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTARootfsNewVersion = 
				iniOtaPackVersionConfig["ROOTFS_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTASystemNewVersion = 
				iniOtaPackVersionConfig["SYSTEM_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTAModelNewVersion = 
				iniOtaPackVersionConfig["MODEL_FW_CONFIG"]["VERSION"].as<std::string>();
			m_strOTAMCUNewVersion = 
				iniOtaPackVersionConfig["MCU_FW_CONFIG"]["VERSION"].as<std::string>();

			m_strOTAPackageFileName = 
				iniOtaPackVersionConfig["APP_FW_CONFIG"]["NAME"].as<std::string>();
			m_strOTAKernelFileName = 
				iniOtaPackVersionConfig["KERNEL_FW_CONFIG"]["NAME"].as<std::string>();
			m_strOTARootfsFileName = 
				iniOtaPackVersionConfig["ROOTFS_FW_CONFIG"]["NAME"].as<std::string>();
			m_strOTASystemFileName = 
				iniOtaPackVersionConfig["SYSTEM_FW_CONFIG"]["NAME"].as<std::string>();
			m_strOTAModelFileName = 
				iniOtaPackVersionConfig["MODEL_FW_CONFIG"]["NAME"].as<std::string>();
			m_strOTAMCUFileName = 
				iniOtaPackVersionConfig["MCU_FW_CONFIG"]["NAME"].as<std::string>();
		}

		catch (std::invalid_argument &ia)
		{
			logPrint(MX_LOG_ERROR, "load version file failed");
			return mxfalse;
		}

		m_bLoadConfig = mxtrue;

#endif
		return mxtrue;
	}

	mxbool COTAManageModule::upgradeCheck()
	{
		logPrint(MX_LOG_INFOR, "enter upgrade check");
		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_CHECK);

		if (!loadVersionConfig())
		{
			logPrint(MX_LOG_ERROR, "load version config failed");
			return mxfalse;
		}

		int iRetCompareRet = versionCompare(m_strOTAPackageVersion, m_strOTAPackageNewVersion);
		if (iRetCompareRet <= 0)
		{
			logPrint(MX_LOG_ERROR, "all package version  is error old: %s new: %s",
				m_strOTAPackageVersion.c_str(),
				m_strOTAPackageNewVersion.c_str());
			return mxfalse;
		}
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 55);

			m_objOTAManageProgressReport->pushData(data);
		}

		logPrint(MX_LOG_INFOR, "upgrade check success, old version:%s, new version:%s", m_strOTAPackageVersion.c_str(), m_strOTAPackageNewVersion.c_str());

		return mxtrue;
	}

	mxbool COTAManageModule::upgradeKernel()
	{
		logPrint(MX_LOG_INFOR, "enter upgrade kernel");
		std::string strValue;

		if (!loadVersionConfig())
		{
			logPrint(MX_LOG_ERROR, "load version config failed");
			return mxfalse;
		}

		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 57);

			m_objOTAManageProgressReport->pushData(data);
		}

		const char *srcFileName = m_strOTAKernelFileName.c_str();
		const char *dstFileName = FW_KERNEL_PARTITION;
		if (access(dstFileName, 0) != 0)
		{
			logPrint(MX_LOG_ERROR, "kernel access version path:%s failed", dstFileName);
			return mxfalse;
		}

		int iRetCompareRet = versionCompare(m_strOTAKernelVersion, m_strOTAKernelNewVersion);
		if (iRetCompareRet <= 0)
		{
			logPrint(MX_LOG_WARN, "kernel version is skip upgrade, old: %s new: %s",
				m_strOTAKernelVersion.c_str(),
				m_strOTAKernelNewVersion.c_str());
			goto skip_unpack_bin;
		}
		logPrint(MX_LOG_INFOR, "begin backup kernel, old version:%s, new version:%s", m_strOTAKernelVersion.c_str(), m_strOTAKernelNewVersion.c_str());

		/// 备份NOR FLASH中的到NAND FLASH
		if (!backupFirmwareBin(FW_KERNEL_PARTITION, FW_KERNEL_BACK_PARTITION))
		{
			logPrint(MX_LOG_ERROR, "back firmware bin failed, name :%s", FW_KERNEL_PARTITION);
			return mxfalse;
		}
		linuxPopenExecCmd(strValue, "tag_env_info --set OTA ota_step %d", E_OTA_UPGRADE_KERNEL);
		logPrint(MX_LOG_INFOR, "backup kernel success, old version:%s, new version:%s", m_strOTAKernelVersion.c_str(), m_strOTAKernelNewVersion.c_str());

		// 新固件烧写到NOR FLASH中
		if (!unpackFirmwareBin(FW_BIN_PATH, srcFileName, dstFileName, 4, mxtrue))
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin failed, name :%s", srcFileName);
			return mxfalse;
		}
		logPrint(MX_LOG_INFOR, "unpack kernel success, old version:%s, new version:%s", m_strOTAKernelVersion.c_str(), m_strOTAKernelNewVersion.c_str());

skip_unpack_bin:
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 60);

			m_objOTAManageProgressReport->pushData(data);
		}

		logPrint(MX_LOG_INFOR, "upgrade kernel success, old version:%s, new version:%s", m_strOTAKernelVersion.c_str(), m_strOTAKernelNewVersion.c_str());
		return mxtrue;
	}

	mxbool COTAManageModule::upgradeRootfs()
	{
		logPrint(MX_LOG_INFOR, "enter upgrade rootfs");

		std::string strValue;

		if (!loadVersionConfig())
		{
			logPrint(MX_LOG_ERROR, "load version config failed");
			return mxfalse;
		}
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 62);
			m_objOTAManageProgressReport->pushData(data);
		}
		const char *srcFileName = m_strOTARootfsFileName.c_str();
		const char *dstFileName = FW_ROOTFS_PARTITION;
		if (access(dstFileName, 0) != 0)
		{
			logPrint(MX_LOG_ERROR, "rootfs access version path:%s failed", dstFileName);
			return mxfalse;
		}
		int iRetCompareRet = versionCompare(m_strOTARootfsVersion, m_strOTARootfsNewVersion);
		if (iRetCompareRet <= 0)
		{
			logPrint(MX_LOG_WARN, "rootfs version is skip upgrade, old: %s new: %s",
				m_strOTARootfsVersion.c_str(),
				m_strOTARootfsNewVersion.c_str());
			goto skip_unpack_bin;
		}
		logPrint(MX_LOG_INFOR, "begin backup rootfs, old version:%s, new version:%s", m_strOTAKernelVersion.c_str(), m_strOTAKernelNewVersion.c_str());
		/// 备份NOR FLASH中的到NAND FLASH
		if (!backupFirmwareBin(FW_ROOTFS_PARTITION, FW_ROOTFS_BACK_PARTITION))
		{
			logPrint(MX_LOG_ERROR, "back firmware bin failed, name :%s", FW_KERNEL_PARTITION);
			return mxfalse;
		}
		linuxPopenExecCmd(strValue, "tag_env_info --set OTA ota_step %d", E_OTA_UPGRADE_ROOTFS);
		logPrint(MX_LOG_INFOR, "backup rootfs success, old version:%s, new version:%s", m_strOTARootfsVersion.c_str(), m_strOTARootfsNewVersion.c_str());

		/// 新固件烧写到NOR FLASH中
		logPrint(MX_LOG_INFOR, "begin unpack rootfs, old version:%s, new version:%s", m_strOTARootfsVersion.c_str(), m_strOTARootfsNewVersion.c_str());
		if (!unpackFirmwareBin(FW_BIN_PATH, srcFileName, dstFileName, 4, mxtrue))
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin failed, name :%s", srcFileName);
			return mxfalse;
		}
		logPrint(MX_LOG_INFOR, "unpack rootfs success, old version:%s, new version:%s", m_strOTARootfsVersion.c_str(), m_strOTARootfsNewVersion.c_str());

skip_unpack_bin:
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 65);

			m_objOTAManageProgressReport->pushData(data);
		}

		logPrint(MX_LOG_INFOR, "upgrade rootfs success, old version:%s, new version:%s", m_strOTARootfsVersion.c_str(), m_strOTARootfsNewVersion.c_str());

		return mxtrue;	
	}

	mxbool COTAManageModule::upgradeSystem()
	{
		logPrint(MX_LOG_INFOR, "enter upgrade system");

		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_SYSTEM);

		if (!loadVersionConfig())
		{
			logPrint(MX_LOG_ERROR, "load version config failed");
			return mxfalse;
		}

		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 67);

			m_objOTAManageProgressReport->pushData(data);
		}

		const char *srcFileName = m_strOTASystemFileName.c_str();
		const char *dstFileName = (m_iSystemPartition == FW_A_PARTITION) ? FW_SYSTEM_B_PARTITION : FW_SYSTEM_A_PARTITION;
		if (access(dstFileName, 0) != 0)
		{
			logPrint(MX_LOG_ERROR, "system access version path:%s failed", dstFileName);
			return mxfalse;
		}

		int iRetCompareRet = versionCompare(m_strOTASystemVersion, m_strOTASystemNewVersion);
		if (iRetCompareRet <= 0)
		{
			logPrint(MX_LOG_WARN, "kernel version is skip upgrade, old: %s new: %s",
				m_strOTASystemVersion.c_str(),
				m_strOTASystemNewVersion.c_str());
			goto skip_unpack_bin;
		}

		logPrint(MX_LOG_INFOR, "begin unpack system, old version:%s, new version:%s", m_strOTASystemVersion.c_str(), m_strOTASystemNewVersion.c_str());
		if (!unpackFirmwareBin(FW_BIN_PATH, srcFileName, dstFileName, 0, mxtrue))
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin failed, name :%s", srcFileName);
			return mxfalse;
		}

skip_unpack_bin:
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 70);

			m_objOTAManageProgressReport->pushData(data);
		}
		logPrint(MX_LOG_INFOR, "upgrade system success, old version:%s, new version:%s", m_strOTASystemVersion.c_str(), m_strOTASystemNewVersion.c_str());

		return mxtrue;	
	}

	mxbool COTAManageModule::upgradeModel()
	{
		logPrint(MX_LOG_INFOR, "enter upgrade model");

		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_MODEL);

		if (!loadVersionConfig())
		{
			logPrint(MX_LOG_ERROR, "load version config failed");
			return mxfalse;
		}

		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 72);

			m_objOTAManageProgressReport->pushData(data);
		}

		const char *srcFileName = m_strOTAModelFileName.c_str();
		const char *dstFileName = (m_iSystemPartition == FW_A_PARTITION) ? FW_MODEL_B_PARTITION : FW_MODEL_A_PARTITION;
		if (access(dstFileName, 0) != 0)
		{
			logPrint(MX_LOG_ERROR, "model access path:%s failed", dstFileName);
			return mxfalse;
		}

		int iRetCompareRet = versionCompare(m_strOTAModelVersion, m_strOTAModelNewVersion);
		if (iRetCompareRet <= 0)
		{
			logPrint(MX_LOG_WARN, "model version is skip upgrade, old: %s new: %s",
				m_strOTAModelVersion.c_str(),
				m_strOTAModelNewVersion.c_str());
			goto skip_unpack_bin;
		}

		logPrint(MX_LOG_INFOR, "begin unpack model, old version:%s, new version:%s", m_strOTASystemVersion.c_str(), m_strOTASystemNewVersion.c_str());
		if (!unpackFirmwareBin(FW_BIN_PATH, srcFileName, dstFileName, 0, mxtrue))
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin failed, name :%s", srcFileName);
			return mxfalse;
		}

skip_unpack_bin:
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 75);

			m_objOTAManageProgressReport->pushData(data);
		}
		logPrint(MX_LOG_INFOR, "upgrade model success, old version:%s, new version:%s", m_strOTAModelVersion.c_str(), m_strOTAModelNewVersion.c_str());

		return mxtrue;	
	}

	mxbool COTAManageModule::upgradeMCU()
	{
		logPrint(MX_LOG_INFOR, "enter upgrade mcu");

		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_MCU);

		if (!loadVersionConfig())
		{
			logPrint(MX_LOG_ERROR, "load version config failed");
			return mxfalse;
		}

		std::stringstream stream;
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 82);

			m_objOTAManageProgressReport->pushData(data);
		}

		const char *srcFileName = m_strOTAMCUFileName.c_str();
		const char *dstFileName = FW_MCU_PARTITION;
		if (access(dstFileName, 0) != 0)
		{
			logPrint(MX_LOG_ERROR, "mcu access path:%s failed", dstFileName);
			return mxfalse;
		}

		int iRetCompareRet = versionCompare(m_strOTAMCUVersion, m_strOTAMCUNewVersion);
		if (iRetCompareRet <= 0)
		{
			logPrint(MX_LOG_WARN, "model version is skip upgrade, old: %s new: %s",
				m_strOTAMCUVersion.c_str(),
				m_strOTAMCUNewVersion.c_str());
			goto skip_unpack_bin;
		}

		logPrint(MX_LOG_INFOR, "begin unpack mcu, old version:%s, new version:%s", m_strOTASystemVersion.c_str(), m_strOTASystemNewVersion.c_str());
		if (!unpackFirmwareBin(FW_BIN_PATH, srcFileName, dstFileName, 0, mxtrue))
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin failed, name :%s", srcFileName);
			return mxfalse;
		}

		/// todo 先将固件刷到mcu这边，成功则再存一份到中继这边，失败则把老的刷回去，先直接刷，之后需要判断是否成功
		stream << "update_fw " << FW_MCU_PARTITION << " 1";
        if (SendToAtCmd(stream.str()) < 0)
		{
			logPrint(MX_LOG_ERROR, "mcu update failed");
			return mxfalse;
		}
		logPrint(MX_LOG_INFOR, "send to at cmd :%s", stream.str().c_str());
        sleep(20);
        SendToAtCmd("fw_cmd \"AT+LIGHT_SLEEP 1\"");

skip_unpack_bin:
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 90);

			m_objOTAManageProgressReport->pushData(data);
		}

		logPrint(MX_LOG_INFOR, "upgrade mcu success, old version:%s, new version:%s", m_strOTAMCUVersion.c_str(), m_strOTAMCUNewVersion.c_str());

		return mxtrue;	
	}

	mxbool COTAManageModule::upgradeComplete()
	{
		std::string strValue;
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_PROGRESS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 0;
			data.strData = reportProcessData("unpack", "200", 99);

			m_objOTAManageProgressReport->pushData(data);
		}

		logPrint(MX_LOG_INFOR, "upgrade compelte success, change to %d partition", !m_iSystemPartition);
		linuxPopenExecCmd(strValue, "tag_env_info --set HW 70mai_system_partition %d", !m_iSystemPartition);
		linuxPopenExecCmd(strValue, "fw_setenv user 70mai_system_partition %d", !m_iSystemPartition);
		linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_COMPLETE);
		linuxPopenExecCmd(strValue, "tag_env_info --set OTA ota_step 0");
		system("touch /tmp/tag_env_info_lock");
		system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
		sleep(3);
		SendToAtCmd("fw_cmd \"AT+REBOOT\"");
		return mxtrue;
	}

	mxbool COTAManageModule::upgradeFailed()
	{
		/// todo 超时定时器停止
		/// m_timerOTATimeout->
		if (m_pFD != NULL)
		{
			fclose(m_pFD);
			m_pFD = NULL;
		}

		if (access(FW_BIN_PATH, 0) == 0)
		{
			remove(FW_BIN_PATH);
		}

		reportOTAFailed();
		
		playAudioFile(FILE_OTA_FAILED);
		setLedStatus(LedInfo_Color_BLUE, LedInfo_State_ON, 0, 0);

		openAIDetect();

		setOTAStage(E_OTA_STA_IDLE);
		std::string strValue;
		linuxPopenExecCmd(strValue, "fw_setenv user ota_sta %d", E_OTA_UPGRADE_IDLE);
		linuxPopenExecCmd(strValue, "fw_setenv user %s %d", LOWPOWER_MODE, ENABLE_LOWPOWER);

		logPrint(MX_LOG_INFOR, "upgrade failed");
		return mxtrue;
	}

	std::string COTAManageModule::reportProcessData(
		std::string strItem, std::string strCode, int iProcess)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "event", "OTAProcess");
		cJSON_AddStringToObject(jsonParam, "item", strItem.c_str());
		cJSON_AddStringToObject(jsonParam, "code", strCode.c_str());
		cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());
		cJSON_AddNumberToObject(jsonParam, "process", iProcess);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
		char *out = cJSON_Print(jsonRoot);
		std::string strData = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return strData;
	}

	void COTAManageModule::reportOTATimeout()
	{
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_STATUS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 3;
			data.strData = reportProcessData("timeout", "400", 0);

			m_objOTAManageProgressReport->pushData(data);
		}
	}

	void COTAManageModule::reportOTAFailed()
	{
		if (m_objOTAManageProgressReport)
		{
			T_OTAReportData data;
			data.eReportType = E_R_STATUS;
			data.lTimeStamp = getCurrentTime() + 10 * 1000;
			data.iTryNum = 3;
			data.strData = reportProcessData("failed", "400", 0);

			m_objOTAManageProgressReport->pushData(data);
		}
	}

	int64_t COTAManageModule::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

	int COTAManageModule::SendToAtCmd(std::string strCommand)
    {
        char command[16000];
        struct sockaddr_un ser_un;
        int socket_fd = 0;
        int ret = 0;

        memset(command, 0, sizeof(command));
        strncpy(command,strCommand.c_str(),strlen(strCommand.c_str()));

        socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socket_fd <= 0)
        {
			logPrint(MX_LOG_ERROR, "open socket error");
            return -1;
        }

        memset(&ser_un, 0, sizeof(ser_un));  
        ser_un.sun_family = AF_UNIX;  
        strcpy(ser_un.sun_path, "/usr/wifi/server_socket");

        ret = ::connect(socket_fd, (struct sockaddr *)&ser_un, sizeof(struct sockaddr_un));  
        if(ret < 0)  
        {  
            logPrint(MX_LOG_ERROR, "connect error");
            return -2; 
        }

        write(socket_fd, command, strlen(command)+1);
        read(socket_fd, command, sizeof(command));
        if (strcmp(command, "OK"))
        {
            logPrint(MX_LOG_ERROR, "send cmd error");
            return -3;
        }

        return 0;
    }

	mxbool COTAManageModule::playAudioFile(std::string strFileId)
	{
		logPrint(MX_LOG_INFOR, "ota play audio:%s", strFileId.c_str());
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
			logPrint(MX_LOG_ERROR, "ota play file json error");
			return mxfalse;
		}

        std::string strRet = output(m_strDevManageGUID, m_strDevManageServer, (unsigned char *)strJson.c_str(),
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
				logPrint(MX_LOG_INFOR, "ota play audio:%s success", strFileId.c_str());
                return mxtrue;
            }
		}

		return mxfalse;
	}

	mxbool COTAManageModule::setLedStatus(int iColor, int iState, int iOnTimeMs, int iOffTimeMs)
	{
		logPrint(MX_LOG_INFOR, "ota set led status, color:%d, state:%d, on_ms:%d, off_ms:%d", 
										iColor, iState, iOnTimeMs, iOffTimeMs);
		std::string strJson;
		cJSON *jsonRoot = cJSON_CreateObject();
		if (jsonRoot)
		{
			cJSON_AddStringToObject(jsonRoot, "event", "SetLedStatus");
			cJSON *jsonParam = cJSON_CreateObject();
			if (jsonParam)
			{        
				cJSON_AddNumberToObject(jsonParam, "led_Color", iColor);
				cJSON_AddNumberToObject(jsonParam, "led_State", iState);
				cJSON_AddNumberToObject(jsonParam, "on_time_ms", iOnTimeMs);
				cJSON_AddNumberToObject(jsonParam, "off_time_ms", iOffTimeMs);

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
			logPrint(MX_LOG_ERROR, "ota set led json error");
			return mxfalse;
		}

        std::string strRet = output(m_strMcuManageGUID, m_strMcuManageServer, (unsigned char *)strJson.c_str(),
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
				logPrint(MX_LOG_INFOR, "ota set led status success, color:%d, state:%d, on_ms:%d, off_ms:%d", 
										iColor, iState, iOnTimeMs, iOffTimeMs);
                return mxtrue;
            }
		}

		return mxfalse;
	}

	std::string COTAManageModule::closeAIDetect()
	{
        cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "closeAIDetect");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);

        char *out = cJSON_PrintUnformatted(pJsonRoot);

		std::string strRet = output(m_strAIGUID, m_strAIServer, (unsigned char *)out, strlen(out) + 1);

		logPrint(MX_LOG_INFOR, "close AI Detect, msg:%s, ret:%s", out, strRet.c_str());

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
	
	std::string COTAManageModule::openAIDetect()
	{
        cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "openAIDetect");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);

        char *out = cJSON_PrintUnformatted(pJsonRoot);

		std::string strRet = output(m_strAIGUID, m_strAIServer, (unsigned char *)out, strlen(out) + 1);

		logPrint(MX_LOG_INFOR, "oepn AI Detect, msg:%s, ret:%s", out, strRet.c_str());

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

}
