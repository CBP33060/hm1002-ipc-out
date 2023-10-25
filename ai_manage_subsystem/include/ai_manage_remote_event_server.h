#ifndef __AI_MANAGE_REMOTE_EVENT_SERVER_H__
#define __AI_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "ai_manage_module.h"

namespace maix {
	class CAIManageRemoteEventServer : public CComProxyBase
	{
	public:
		CAIManageRemoteEventServer(CAIManageModule
			*objAIManageModule);
		~CAIManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CAIManageModule *m_objAIManageModule;
	};
}
#endif //__AI_MANAGE_REMOTE_EVENT_SERVER_H__
