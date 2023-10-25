#ifndef __LOW_POWER_INFORM_FLOW_H__
#define __LOW_POWER_INFORM_FLOW_H__
#include "module.h"

#define IPC_MANAGE_EXIT		(0x00000001)
#define DEV_MANAGE_EXIT		(0x00000002)
#define EVENT_MANAGE_EXIT	(0x00000004)
#define VIDEO_MANAGE_EXIT	(0x00000008)
#define AUDIO_MANAGE_EXIT	(0x00000010)
#define SPEAKER_MANAGE_EXIT	(0x00000020)
#define LOG_MANAGE_EXIT		(0x00000040)
#define VIDEO_SOURCE_EXIT	(0x00000080)

#define ALL_MODULE_EXIT (IPC_MANAGE_EXIT | \
            DEV_MANAGE_EXIT | \
            EVENT_MANAGE_EXIT | \
            AUDIO_MANAGE_EXIT |\
            SPEAKER_MANAGE_EXIT | \
            LOG_MANAGE_EXIT | \
            VIDEO_SOURCE_EXIT)

#define LOWPOWER_MODE							    "lowpower_mode"
#define ENABLE_LOWPOWER								1
#define DISABLE_LOWPOWER							0

namespace maix {
	typedef enum
	{
		E_Module_IDLE,
		E_IPCManageModule,
		E_DevManageModule,
		E_EventManageModule,
		E_VideoManageModule,
		E_AudioManageModule,
		E_SpeakerManageModule,
        E_LogManageModule,
        E_VideoSourceModule,
		E_EnterLowPower,
	}E_Module_Type;

	class  CLowPowerInformFlow
	{
	public:
		CLowPowerInformFlow(CModule * module);
		~CLowPowerInformFlow();
		mxbool init();
		mxbool unInit();
		void run();

		E_Module_Type getModuleType();
		mxbool setModuleType(E_Module_Type eType);

		mxbool sendIPCManageModuleEnterLowPower();
		mxbool sendDevManageModuleEnterLowPower();
		mxbool sendEventManageModuleEnterLowPower();
		mxbool sendVideoManageModuleEnterLowPower();
		mxbool sendAudioManageModuleEnterLowPower();
		mxbool sendSpeakerManageModuleEnterLowPower();
        mxbool sendLogManageModuleEnterLowPower();
        mxbool sendMediaSourceModuleEnterLowPower();
		mxbool enterLowPower(E_Module_Type eType);
		mxbool lowPowerProc();
	private:
		CModule * m_module;
		volatile E_Module_Type m_eModuleType;
		int m_iModuleExit;
		
		std::string m_strIPCManageModuleGUID;
		std::string m_strIPCManageModuleServer;

		std::string m_strDevManageModuleGUID;
		std::string m_strDevManageModuleServer;

		std::string m_strEventManageModuleGUID;
		std::string m_strEventManageModuleServer;

		std::string m_strVideoManageModuleGUID;
		std::string m_strVideoManageModuleServer;

		std::string m_strAudioManageModuleGUID;
		std::string m_strAudioManageModuleServer;

		std::string m_strSpeakerManageModuleGUID;
		std::string m_strSpeakerManageModuleServer;

		std::string m_strMCUManageModuleGUID;
		std::string m_strMCUManageModuleServer;

        std::string m_strLogManageModuleGUID;
        std::string m_strLogManageModuleServer;

        std::string m_strMediaSourceModuleGUID;
        std::string m_strMediaSourceModuleServer;

		mxbool checkLowPowerMode();

        int m_iEnterProcCount;
	};
}
#endif //__LOW_POWER_INFORM_FLOW_H__
