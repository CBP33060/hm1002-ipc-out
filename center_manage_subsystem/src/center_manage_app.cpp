#include "center_manage_app.h"
#include "log_mx.h"
#include "center_manage_module.h"
#include "low_power_module.h"

namespace maix {
	CCenterManageApp::CCenterManageApp(std::string strName)
		: CApp(strName)
	{
	}

	CCenterManageApp::~CCenterManageApp()
	{
	}

	mxbool CCenterManageApp::init()
	{
		T_LogConfig tLogConfig;
		std::string strName;
		if (!getConfig("APP", "NAME", strName))
		{
			std::cout << "[CenterManageApp]: get app name failed" << std::endl; 
			return mxfalse;
		}

		mxbool bLogEnable = mxfalse;
		if (!getConfig("APP", "LOG_ENABLE", bLogEnable))
		{
			std::cout << "[CenterManageApp]: get app log enable failed" << std::endl; 
			return mxfalse;
		}

		int iType = MX_LOG_NULL;
		if (!getConfig("LOG_CONFIG", "TYPE", iType))
		{
			std::cout << "[CenterManageApp]: get log config type failed" << std::endl;
			return mxfalse;
		}

		int iLevel = MX_LOG_ERROR;
		if (!getConfig("LOG_CONFIG", "LEVEL", iLevel))
		{
			std::cout << "[CenterManageApp]: get log config level failed" << std::endl;
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
            	std::cout << "[CenterManageApp]: get log config file name failed" << std::endl;
                return mxfalse;
            }

            tLogConfig.m_strFileName = strFileName;
            std::string strUnix;
            if (!getConfig("LOG_CONFIG", "UNIX", strUnix))
            {
            	std::cout << "[CenterManageApp]: get log config unix failed" << std::endl;
                return mxfalse;
            }

            tLogConfig.m_strUnix = strUnix;
        }
		else if (tLogConfig.m_eType == MX_LOG_REMOTE)
		{
			int iNetType = MX_LOG_TCP;
			if (!getConfig("LOG_CONFIG", "NET_TYPE", iNetType))
			{
				std::cout << "[CenterManageApp]: get log config net type failed" << std::endl;
				return mxfalse;
			}

			tLogConfig.m_eNetType = (E_MX_LOG_NET_TYPE)iNetType;

			std::string strIP;
			if (!getConfig("LOG_CONFIG", "IP", strIP))
			{
				std::cout << "[CenterManageApp]: get log config ip failed" << std::endl;
				return mxfalse;
			}
			tLogConfig.m_strIP = strIP;

			int iPort = 0;
			if (!getConfig("LOG_CONFIG", "PORT", iPort))
			{
				std::cout << "[CenterManageApp]: get log config port failed" << std::endl;
				return mxfalse;
			}
			tLogConfig.m_iPort = iPort;
		}

		logInit(tLogConfig);

		if (!getConfig("CENTER_CONFIG", "NAME", m_strCenterMoudleName))
		{
			std::cout << "[CenterManageApp]: get center config name failed" << std::endl;
			return mxfalse;
		}

		if (!getConfig("CENTER_CONFIG", "GUID", m_strCenterMoudleGUID))
		{
			std::cout << "[CenterManageApp]: get center config guid failed" << std::endl;
			return mxfalse;
		}

		std::string strCenterConfigPath;
		if (!getConfig("CENTER_CONFIG", "CONFIG", strCenterConfigPath))
		{
			std::cout << "[CenterManageApp]: get center config path failed" << std::endl;
			return mxfalse;
		}

		if (!initCenterManage(strCenterConfigPath))
		{
			std::cout << "[CenterManageApp]: init center manage failed" << std::endl;
			return mxfalse;
		}

		if (!getConfig("LOW_POWER_CONFIG", "NAME", m_strLowPowerMoudleName))
		{
			std::cout << "[CenterManageApp]: get low power config name failed" << std::endl;
			return mxfalse;
		}

		if (!getConfig("LOW_POWER_CONFIG", "GUID", m_strLowPowerMoudleGUID))
		{
			std::cout << "[CenterManageApp]: get low power config guid failed" << std::endl;
			return mxfalse;
		}

		std::string strLowPowerConfigPath;
		if (!getConfig("LOW_POWER_CONFIG", "CONFIG", strLowPowerConfigPath))
		{
			std::cout << "[CenterManageApp]: get low power config path failed" << std::endl;
			return mxfalse;
		}

		if (!initLowPowerManage(strLowPowerConfigPath))
		{
			std::cout << "[CenterManageApp]: init low power manage failed" << std::endl;
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CCenterManageApp::unInit()
	{
		return mxbool();
	}

	mxbool CCenterManageApp::initCenterManage(std::string strConfigPath)
	{
		std::shared_ptr<CCenterManageModule> objCenterManageModule(
			new CCenterManageModule(m_strCenterMoudleGUID, 
				m_strCenterMoudleName));
		if (!objCenterManageModule->loadConfig(strConfigPath))
		{
			std::cout << "[CenterManageApp]: init center manage load config failed" << std::endl;
			return mxfalse;
		}
			

		if (!objCenterManageModule->init())
		{
			std::cout << "[CenterManageApp]: init center manage init failed" << std::endl;
			return mxfalse;
		}
			

		return addModule(m_strCenterMoudleName, objCenterManageModule);
	}

	mxbool CCenterManageApp::unInitCenterManage()
	{
		return mxbool();
	}
	mxbool CCenterManageApp::initLowPowerManage(
		std::string strConfigPath)
	{
		std::shared_ptr<CLowPowerModule> objLowPowerModule(
			new CLowPowerModule(m_strLowPowerMoudleGUID,
				m_strLowPowerMoudleName));
		if (!objLowPowerModule->loadConfig(strConfigPath))
		{
			std::cout << "[CenterManageApp]: init low power manage load config failed" << std::endl;
			return mxfalse;
		}

		if (!objLowPowerModule->init())
		{
			std::cout << "[CenterManageApp]: init low power manage init failed" << std::endl;
			return mxfalse;
		}

		return addModule(m_strLowPowerMoudleName, objLowPowerModule);
	}

	mxbool CCenterManageApp::unInitLowPowerManage()
	{
		return mxbool();
	}
}
