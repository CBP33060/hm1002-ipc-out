#ifndef __SPEAKER_SOURCE_REMOTE_EVENT_SERVER_H__
#define __SPEAKER_SOURCE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "speaker_source_module.h"

namespace maix {
	class CSpeakerSourceRemoteEventServer : public CComProxyBase
	{
	public:
		CSpeakerSourceRemoteEventServer(CSpeakerSourceModule
			*objSpeakerSourceModule);
		~CSpeakerSourceRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CSpeakerSourceModule *m_objSpeakerSourceModule;
	};
}
#endif //__SPEAKER_SOURCE_REMOTE_EVENT_SERVER_H__
