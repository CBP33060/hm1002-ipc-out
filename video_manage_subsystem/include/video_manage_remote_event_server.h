#ifndef __VIDEO_MANAGE_REMOTE_EVENT_SERVER_H__
#define __VIDEO_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "video_manage_module.h"

namespace maix {
	class CVideoManageRemoteEventServer : public CComProxyBase
	{
	public:
		CVideoManageRemoteEventServer(CVideoManageModule
			*objVideoManageModule);
		~CVideoManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CVideoManageModule *m_objVideoManageModule;
	};
}
#endif //__VIDEO_MANAGE_REMOTE_EVENT_SERVER_H__
