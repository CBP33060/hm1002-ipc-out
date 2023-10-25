#ifndef __DEV_MANAGE_REMOTE_EVENT_SERVER_H__
#define __DEV_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "dev_manage_module.h"

namespace maix {
	class CDevManageRemoteEventServer : public CComProxyBase
	{
	public:
		CDevManageRemoteEventServer(CDevManageModule
			*objDevManageModule);
		~CDevManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

	private:
		CDevManageModule *m_objDevManageModule;
	};
}
#endif //__DEV_MANAGE_REMOTE_EVENT_SERVER_H__
