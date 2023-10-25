#include "audio_module_local_event_server.h"

namespace maix {
	CAudioModuleLocalEventServer::CAudioModuleLocalEventServer()
	{
	}

	CAudioModuleLocalEventServer::~CAudioModuleLocalEventServer()
	{
	}

	std::string CAudioModuleLocalEventServer::msgHandle(
		unsigned char * pcData, int iLen)
	{
		printf("audio module: %s\n", pcData);
		return std::string();
	}
}
