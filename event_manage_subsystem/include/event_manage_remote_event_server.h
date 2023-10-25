#ifndef __EVENT_MANAGE_REMOTE_EVENT_SERVER_H__
#define __EVENT_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "event_manage_module.h"

namespace maix {
	class CEventManageRemoteEventServer : public CComProxyBase
	{
	public:
		CEventManageRemoteEventServer(CEventManageModule
			*objEventManageModule);
		~CEventManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

	private:
		CEventManageModule *m_objEventManageModule;
	};
}
#endif //__EVENT_MANAGE_REMOTE_EVENT_SERVER_H__
