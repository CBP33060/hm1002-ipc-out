#include "video_module_local_event_server.h"
#ifdef WIN32
#include <windows.h>
#endif
namespace maix {
	CVideoModuleLocalEventServer::CVideoModuleLocalEventServer()
	{
	}

	CVideoModuleLocalEventServer::~CVideoModuleLocalEventServer()
	{
	}

	std::string CVideoModuleLocalEventServer::msgHandle(unsigned char * pcData, int iLen)
	{
		printf("video module: %s\n", pcData);
	
		return std::string("nihao");
	}
}