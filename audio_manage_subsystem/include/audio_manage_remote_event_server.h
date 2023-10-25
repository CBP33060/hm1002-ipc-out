#ifndef __AUDIO_MANAGE_REMOTE_EVENT_SERVER_H__
#define __AUDIO_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "audio_manage_module.h"

namespace maix {
	class CAudioManageRemoteEventServer : public CComProxyBase
	{
	public:
		CAudioManageRemoteEventServer(CAudioManageModule
			*objAudioManageModule);
		~CAudioManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CAudioManageModule *m_objAudioManageModule;
	};
}
#endif //__AUDIO_MANAGE_REMOTE_EVENT_SERVER_H__
