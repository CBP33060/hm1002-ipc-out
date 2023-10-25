#ifndef __CENTER_MANAGE_REMOTE_EVENT_SERVER_H__
#define __CENTER_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "center_manage_module.h"

namespace maix {
	class CCenterManageRemoteEventServer : public CComProxyBase
	{
	public:
		CCenterManageRemoteEventServer(CCenterManageModule
			*objCenterManageModule);
		~CCenterManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CCenterManageModule *m_objCenterManageModule;
	};
}
#endif //__CENTER_MANAGE_REMOTE_EVENT_SERVER_H__