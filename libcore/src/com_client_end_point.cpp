#include "com_client_end_point.h"

namespace maix {
	CComClientEndPoint::CComClientEndPoint(CModule* module)
		: m_module(module)
	{
	}

	std::string CComClientEndPoint::output(std::string strServerName,
		unsigned char * pcData, int iLen)
	{
		std::shared_ptr<CIComServerEndPoint> server = m_module->getServer(strServerName);
		
		return server->input(pcData, iLen);
	}

	int CComClientEndPoint::getClientType()
	{
		return 0;
	}
}