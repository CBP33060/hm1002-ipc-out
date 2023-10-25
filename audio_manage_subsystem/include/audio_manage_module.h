#ifndef __AUDIO_MANAGE_MODULE_H__
#define __AUDIO_MANAGE_MODULE_H__
#include "module.h"
#include "audio_source_input_server.h"
#include "audio_manage_channel.h"
#include "audio_manage_channel_session.h"

namespace maix {
	class MAIX_EXPORT CAudioManageModule : public CModule
	{
	public:
		CAudioManageModule(std::string strGUID, std::string strName);
		~CAudioManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		void   lowPowerRun();
		std::string lowPowerProc();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		mxbool addAudioChannel(std::string strName,
			CAudioSourceInputServer* objAudioSourceInputServer);

		std::string openAudio(std::string strParam);
		std::string closeAudio(std::string strParam);
		std::string configAudio(std::string strParam);
		std::string resetAudio(std::string strParam);
		std::string enterLowPower(std::string strParam);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		std::mutex m_mutexAudioChn;
		std::map<std::string, std::thread> m_mapAudioChnProc;
		std::map<std::string, std::shared_ptr<CAudioManageChannel>> m_mapAudioChn;
		std::map<std::string, std::thread> m_mapAudioChnSessionProc;
		std::map<std::string, std::shared_ptr<CAudioManageChannelSession>> m_mapAudioChnSession;

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;
	};
}
#endif //__AUDIO_MANAGE_MODULE_H__
