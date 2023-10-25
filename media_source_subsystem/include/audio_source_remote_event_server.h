#ifndef __AUDIO_SOURCE_REMOTE_EVENT_SERVER_H__
#define __AUDIO_SOURCE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "audio_source_module.h"

namespace maix {
	class CAudioSourceRemoteEventServer : public CComProxyBase
	{
	public:
		CAudioSourceRemoteEventServer(CAudioSourceModule
			*objAudioSourceModule);
		~CAudioSourceRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CAudioSourceModule *m_objAudioSourceModule;
	};
}
#endif //__AUDIO_SOURCE_REMOTE_EVENT_SERVER_H__
