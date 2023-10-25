#ifndef __SPEAKER_MANAGE_MODULE_H__
#define __SPEAKER_MANAGE_MODULE_H__
#include "module.h"
#include "speaker_source_input_server.h"
#include "speaker_manage_channel.h"

namespace maix {
	class MAIX_EXPORT CSpeakerManageModule : public CModule
	{
	public:
		CSpeakerManageModule(std::string strGUID, std::string strName);
		~CSpeakerManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		void   lowPowerRun();
		std::string lowPowerProc();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		mxbool addSpeakerChannel(std::string strName,
			CSpeakerSourceInputServer* objSpeakerSourceInputServer);

		std::string open(std::string strParam);
		std::string close(std::string strParam);
		std::string config(std::string strParam);
		std::string reset(std::string strParam);
		std::string enterLowPower(std::string strParam);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:

        mxbool initSpeakerSourceRemote();
        mxbool sendSpeakerSourceUninit();

		std::mutex m_mutexSpeakerChn;
		std::map<std::string, std::thread> m_mapSpeakerChnProc;
		std::map<std::string, 
			std::shared_ptr<CSpeakerManageChannel>> m_mapSpeakerChn;

		std::mutex m_mutexSpeakerInputServer;
		std::map<std::string, CSpeakerSourceInputServer*>
			m_mapSpeakerInputServer;

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

        std::string m_strSpeakerSourceGUID;
        std::string m_strSpeakerSourceServer;
	};
}
#endif //__SPEAKER_MANAGE_MODULE_H__
