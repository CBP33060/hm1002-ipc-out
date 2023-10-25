#ifndef __VIDEO_MANAGE_MODULE_H__
#define __VIDEO_MANAGE_MODULE_H__
#include "module.h"
#include "video_source_input_server.h"
#include "video_manage_channel.h"
#include "video_manage_channel_session.h"

namespace maix {
	class MAIX_EXPORT CVideoManageModule : public CModule
	{
	public:
		CVideoManageModule(std::string strGUID, std::string strName);
		~CVideoManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		void   lowPowerRun();
		std::string lowPowerProc();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		mxbool addVideoChannel(std::string strName,
			CVideoSourceInputServer* objVideoSourceInputServer);

		std::string openVideo(std::string strParam);
		std::string closeVideo(std::string strParam);
		std::string configVideo(std::string strParam);
		std::string resetVideo(std::string strParam);
		std::string enterLowPower(std::string strParam);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

	private:
		std::mutex m_mutexVideoChn;
		std::map<std::string, std::thread> m_mapVideoChnProc;
		std::map<std::string, std::shared_ptr<CVideoManageChannel>> m_mapVideoChn;
		std::map<std::string, std::thread> m_mapVideoChnSessionProc;
		std::map<std::string, std::shared_ptr<CVideoManageChannelSession>> m_mapVideoChnSession;

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

		std::mutex m_mutexLiveStreamNum;
		int m_iLiveStreamNum;
	};
}
#endif //__VIDEO_MANAGE_MODULE_H__
