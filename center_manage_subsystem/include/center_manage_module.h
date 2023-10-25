#ifndef __CENTER_MANAGE_MODULE_H__
#define __CENTER_MANAGE_MODULE_H__
#include "module.h"

namespace maix {
	class MAIX_EXPORT CCenterManageModule : public CModule
	{
	public:
		CCenterManageModule(std::string strGUID, std::string strName);
		~CCenterManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		
	};
}
#endif //__CENTER_MANAGE_MODULE_H__
