#ifndef __SPEAKER_MANAGE_REMOTE_EVENT_SERVER_H__
#define __SPEAKER_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "speaker_manage_module.h"

namespace maix {
	class CSpeakerManageRemoteEventServer : public CComProxyBase
	{
	public:
		CSpeakerManageRemoteEventServer(CSpeakerManageModule
			*objSpeakerManageModule);
		~CSpeakerManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CSpeakerManageModule *m_objSpeakerManageModule;
	};
}
#endif //__SPEAKER_MANAGE_REMOTE_EVENT_SERVER_H__
