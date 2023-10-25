#include "com_server_end_point.h"

namespace maix {
	CComServerEndPoint::CComServerEndPoint(CModule* module)
		: m_module(module)
	{
	}

	mxbool CComServerEndPoint::init(std::shared_ptr<CIComServerHandle> handle)
	{
		m_handle = handle;
		return mxtrue;
	}

	std::string CComServerEndPoint::input(
		unsigned char * pcData, int iLen)
	{
		return m_handle->msgHandle(pcData, iLen);
	}
}