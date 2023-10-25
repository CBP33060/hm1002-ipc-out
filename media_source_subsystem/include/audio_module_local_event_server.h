#ifndef __AUDIO_MODULE_LOCAL_EVENT_SERVER_H__
#define __AUDIO_MODULE_LOCAL_EVENT_SERVER_H__
#include "i_com_server_handle.h"

namespace maix {
	class CAudioModuleLocalEventServer : public CIComServerHandle
	{
	public:
		CAudioModuleLocalEventServer();
		~CAudioModuleLocalEventServer();

		std::string msgHandle(unsigned char * pcData, int iLen);
	};
}
#endif //__AUDIO_MODULE_LOCAL_EVENT_SERVER_H__
