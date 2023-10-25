#include "log_manage_app.h"
#include "log_mx.h"
#include "log_manage_module.h"

namespace maix {
	CLogManageApp::CLogManageApp(std::string strName)
		: CApp(strName)
	{
	}

	CLogManageApp::~CLogManageApp()
	{
	}

	mxbool CLogManageApp::init()
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

		if (!getConfig("LOG_MODULE_CONFIG", "NAME", m_strLogModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("LOG_MODULE_CONFIG", "GUID", m_strLogModuleGUID))
		{
			return mxfalse;
		}

		std::string strLogConfigPath;
		if (!getConfig("LOG_MODULE_CONFIG", "CONFIG", strLogConfigPath))
		{
			return mxfalse;
		}

		if (!initLogManage(strLogConfigPath))
		{
			return mxfalse;
		}        

        // int count = 0;
        // while (count < 100)
        // {
        //     // mcu_logPrint(MX_LOG_INFOR,"i am mcu   mcu   muc   %d ",count);
        //     mcu_logPrint("i am mcu   mcu   muc   %d ",count);
        //     count++;
        // }
        

		return mxtrue;
	}

	mxbool CLogManageApp::unInit()
	{
		return mxbool();
	}

	mxbool CLogManageApp::initLogManage(std::string strConfigPath)
	{
		std::shared_ptr<CLogManageModule> objLogManageModule(
			new CLogManageModule(m_strLogModuleGUID, 
				m_strLogModuleName));
		if (!objLogManageModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objLogManageModule->init())
			return mxfalse;

		return addModule(m_strLogModuleName, objLogManageModule);
	}

	mxbool CLogManageApp::unInitLogManage()
	{
		return mxbool();
	}
}