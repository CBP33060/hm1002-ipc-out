#include "video_manage_app.h"
#include "log_mx.h"
#include "video_manage_module.h"

namespace maix {
	CVideoManageApp::CVideoManageApp(std::string strName)
		: CApp(strName)
	{
	}

	CVideoManageApp::~CVideoManageApp()
	{
	}

	mxbool CVideoManageApp::init()
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

		if (!getConfig("VIDEO_CONFIG", "NAME", m_strVideoModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("VIDEO_CONFIG", "GUID", m_strVideoModuleGUID))
		{
			return mxfalse;
		}

		std::string strVideoConfigPath;
		if (!getConfig("VIDEO_CONFIG", "CONFIG", strVideoConfigPath))
		{
			return mxfalse;
		}

		if (!initVideoManage(strVideoConfigPath))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CVideoManageApp::unInit()
	{
		return mxbool();
	}

	mxbool CVideoManageApp::initVideoManage(std::string strConfigPath)
	{
		std::shared_ptr<CVideoManageModule> objVideoManageModule(
			new CVideoManageModule(m_strVideoModuleGUID, m_strVideoModuleName));
		if (!objVideoManageModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objVideoManageModule->init())
			return mxfalse;

		return addModule(m_strVideoModuleName, objVideoManageModule);
	}

	mxbool CVideoManageApp::unInitVideoManage()
	{
		return mxbool();
	}
}