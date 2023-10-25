#ifndef __VIDEO_SOURCE_REMOTE_EVENT_SERVER_H__
#define __VIDEO_SOURCE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "video_source_module.h"

namespace maix {
	class CVideoSourceRemoteEventServer : public CComProxyBase
	{
	public:
		CVideoSourceRemoteEventServer(CVideoSourceModule
			*objVideoSourceModule);
		~CVideoSourceRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CVideoSourceModule *m_objVideoSourceModule;
	};
}
#endif //__VIDEO_SOURCE_REMOTE_EVENT_SERVER_H__