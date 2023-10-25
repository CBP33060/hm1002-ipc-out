#ifndef __LOG_MANAGE_REMOTE_EVENT_SERVER_H__
#define __LOG_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "log_manage_module.h"

namespace maix {
	class CLogManageRemoteEventServer : public CComProxyBase
	{
	public:
		CLogManageRemoteEventServer(CLogManageModule
			*objLogManageModule);
		~CLogManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CLogManageModule *m_objLogManageModule;
	};
}
#endif //__LOG_MANAGE_REMOTE_EVENT_SERVER_H__
