#ifndef __VIDEO_MODULE_LOCAL_EVENT_SERVER_H__
#define __VIDEO_MODULE_LOCAL_EVENT_SERVER_H__
#include "i_com_server_handle.h"

namespace maix {
	class CVideoModuleLocalEventServer : public CIComServerHandle
	{
	public:
		CVideoModuleLocalEventServer();
		~CVideoModuleLocalEventServer();

		std::string msgHandle(unsigned char * pcData, int iLen);
	};
}
#endif //__VIDEO_MODULE_LOCAL_EVENT_SERVER_H__
