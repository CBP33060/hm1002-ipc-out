#include "local_storage_app.h"
#include "log_mx.h"
#include "local_storage_module.h"

namespace maix {
	CLocalStorageApp::CLocalStorageApp(std::string strName)
		: CApp(strName)
	{
	}

	CLocalStorageApp::~CLocalStorageApp()
	{
	}

	mxbool CLocalStorageApp::init()
	{
		T_LogConfig tLogConfig;
		std::string strName;
		if (!getConfig("APP", "NAME", strName))
		{
			return mxfalse;
		}

		mxbool bLogEnable = mxfalse;
		if (!getConfig("APP", "LOG_ENABLE", bLogEnable))
		{
			return mxfalse;
		}

		int iType = MX_LOG_NULL;
		if (!getConfig("LOG_CONFIG", "TYPE", iType))
		{
			return mxfalse;
		}

		int iLevel = MX_LOG_ERROR;
		if (!getConfig("LOG_CONFIG", "LEVEL", iLevel))
		{
			return mxfalse;
		}

		tLogConfig.m_strName = strName;
		tLogConfig.m_eType = (E_MX_LOG_TYPE)iType;
		tLogConfig.m_eLevel = (E_MX_LOG_LEVEL)iLevel;

        if ((tLogConfig.m_eType == MX_LOG_LOCAL) || (tLogConfig.m_eType == MX_LOG_CONSOLE_AND_LOCAL))
        {
            std::string strFileName;
            if (!getConfig("LOG_CONFIG", "FILE_NAME", strFileName))
            {
                return mxfalse;
            }

            tLogConfig.m_strFileName = strFileName;
            std::string strUnix;
            if (!getConfig("LOG_CONFIG", "UNIX", strUnix))
            {
                return mxfalse;
            }

            tLogConfig.m_strUnix = strUnix;
        }
		else if (tLogConfig.m_eType == MX_LOG_REMOTE)
		{
			int iNetType = MX_LOG_TCP;
			if (!getConfig("LOG_CONFIG", "NET_TYPE", iNetType))
			{
				return mxfalse;
			}

			tLogConfig.m_eNetType = (E_MX_LOG_NET_TYPE)iNetType;

			std::string strIP;
			if (!getConfig("LOG_CONFIG", "IP", strIP))
			{
				return mxfalse;
			}
			tLogConfig.m_strIP = strIP;

			int iPort = 0;
			if (!getConfig("LOG_CONFIG", "PORT", iPort))
			{
				return mxfalse;
			}
			tLogConfig.m_iPort = iPort;
		}

		logInit(tLogConfig);

		if (!getConfig("STORAGE_CONFIG", "NAME", m_strLocalStorageModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("STORAGE_CONFIG", "GUID", 
			m_strLocalStorageModuleGUID))
		{
			return mxfalse;
		}

		std::string strLocalStorageConfigPath;
		if (!getConfig("STORAGE_CONFIG", "CONFIG", 
			strLocalStorageConfigPath))
		{
			return mxfalse;
		}

		if (!initLocalStorageModule(strLocalStorageConfigPath))
		{
			return mxfalse;
		}
		return mxtrue;
	}

	mxbool CLocalStorageApp::unInit()
	{
		return mxbool();
	}

	mxbool CLocalStorageApp::initLocalStorageModule(std::string strConfigPath)
	{
		std::shared_ptr<CLocalStorageModule> objLocalStorageModule(
			new CLocalStorageModule(m_strLocalStorageModuleGUID,
				m_strLocalStorageModuleName));
		if (!objLocalStorageModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objLocalStorageModule->init())
			return mxfalse;

#if 0
		//for test
		
#ifdef _WIN32
		Sleep(3 * 1000);
#else
		sleep(3);
#endif
		objLocalStorageModule->eventOccurrence("");
		///////////////////////////////
#endif
		return addModule(m_strLocalStorageModuleName, objLocalStorageModule);
	}

	mxbool CLocalStorageApp::unInitLocalStorageModule()
	{
		return mxbool();
	}
}
