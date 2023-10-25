#include "media_source_app.h"
#include "log_mx.h"
#include "media_interface_factory.h"
#include "video_source_module.h"
#include "audio_source_module.h"
#include "speaker_source_module.h"
#include "mcu_serial_port_module.h"
#include "power_up_sound.h"

namespace maix {
	CMediaSourceApp::CMediaSourceApp(std::string strName)
		: CApp(strName)
	{
	}

	CMediaSourceApp::~CMediaSourceApp()
	{
	}

	mxbool CMediaSourceApp::init()
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

		std::string strMediaInterfacePath;
		if (!getConfig("MEDIA_INTERFACE", "PATH", strMediaInterfacePath))
		{
			return mxfalse;
		}
		
		CMediaInterfaceFactory::loadMediaSourceInterface(
			strMediaInterfacePath);

		if (!getConfig("VIDEO_CONFIG", "NAME", m_strVideoModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("AUDIO_CONFIG", "NAME", m_strAudioModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("SPEAKER_CONFIG", "NAME", m_strSpeakerModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("MCU_SERIAL_PORT_CONFIG", "NAME", 
			m_strMCUSerialPortModuleName))
		{
			return mxfalse;
		}

		if (!getConfig("VIDEO_CONFIG", "GUID", m_strVideoGUID))
		{
			return mxfalse;
		}

		if (!getConfig("AUDIO_CONFIG", "GUID", m_strAudioGUID))
		{
			return mxfalse;
		}

		if (!getConfig("SPEAKER_CONFIG", "GUID", m_strSpeakerGUID))
		{
			return mxfalse;
		}

		if (!getConfig("MCU_SERIAL_PORT_CONFIG", "GUID", m_strMCUSerialPortGUID))
		{
			return mxfalse;
		}

		std::string strVideoConfigPath;
		if (!getConfig("VIDEO_CONFIG", "CONFIG", strVideoConfigPath))
		{
			return mxfalse;
		}

		std::string strAudioConfigPath;
		if (!getConfig("AUDIO_CONFIG", "CONFIG", strAudioConfigPath))
		{
			return mxfalse;
		}

		std::string strSpeakerConfigPath;
		if (!getConfig("SPEAKER_CONFIG", "CONFIG", strSpeakerConfigPath))
		{
			return mxfalse;
		}

		std::string strSerialPortConfigPath;
		if (!getConfig("MCU_SERIAL_PORT_CONFIG", "CONFIG", strSerialPortConfigPath))
		{
			return mxfalse;
		}

		if (!initVideoSource(strVideoConfigPath))
		{
			return mxfalse;
		}

		if (!initAudioSource(strAudioConfigPath))
		{
			return mxfalse;
		}

		if (!initSpeakerSource(strSpeakerConfigPath))
		{
			return mxfalse;
		}
		
		if (!initMcuSerialPort(strSerialPortConfigPath))
		{
			return mxfalse;
		}
		
		if (!connect(m_strAudioModuleName, m_strVideoModuleName))
		{
			return mxfalse;
		}

        CPowerUpSound::getInstance().init(this);
        // CPowerUpSound::getInstance().startPlaySound();

		return mxtrue;
	}

	mxbool CMediaSourceApp::unInit()
	{
		unInitVideoSource();
		unInitAudioSource();
		unInitSpeakerSource();
		return mxtrue;
	}

	mxbool CMediaSourceApp::initVideoSource(std::string strConfigPath)
	{
		std::shared_ptr<CVideoSourceModule> objVideoSourceModule(
			new CVideoSourceModule(m_strVideoGUID, 
				m_strVideoModuleName));
		if (!objVideoSourceModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objVideoSourceModule->init())
			return mxfalse;

		return addModule(m_strVideoModuleName, objVideoSourceModule);
	}

	mxbool CMediaSourceApp::unInitVideoSource()
	{
		return delModule(m_strAudioModuleName);
	}

	mxbool CMediaSourceApp::initAudioSource(std::string strConfigPath)
	{
		std::shared_ptr<CAudioSourceModule> objAudioSourceModule(
			new CAudioSourceModule(m_strAudioGUID, 
				m_strAudioModuleName));
		if (!objAudioSourceModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objAudioSourceModule->init())
			return mxfalse;

		return addModule(m_strAudioModuleName, objAudioSourceModule);
	}

	mxbool CMediaSourceApp::unInitAudioSource()
	{
		return mxbool();
	}

	mxbool CMediaSourceApp::initSpeakerSource(std::string strConfigPath)
	{
		std::shared_ptr<CSpeakerSourceModule> objSpeakerSourceModule(
			new CSpeakerSourceModule(m_strSpeakerGUID,
				m_strSpeakerModuleName));
		if (!objSpeakerSourceModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objSpeakerSourceModule->init())
			return mxfalse;

		return addModule(m_strSpeakerModuleName, objSpeakerSourceModule);
	}

	mxbool CMediaSourceApp::unInitSpeakerSource()
	{
		return mxbool();
	}

	mxbool CMediaSourceApp::initMcuSerialPort(std::string strConfigPath)
	{
		std::shared_ptr<CMCUSerialPortModule> objMCUSerialPortModule(
			new CMCUSerialPortModule(m_strMCUSerialPortGUID,
				m_strMCUSerialPortModuleName));
		if (!objMCUSerialPortModule->loadConfig(strConfigPath))
			return mxfalse;

		if (!objMCUSerialPortModule->init())
			return mxfalse;

		return addModule(m_strMCUSerialPortModuleName, objMCUSerialPortModule);
	}

	mxbool CMediaSourceApp::unInitMcuSerialPort()
	{
		return mxbool();
	}

}