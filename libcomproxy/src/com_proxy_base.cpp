#include "com_proxy_base.h"

namespace maix {
	CComProxyBase::CComProxyBase()
	{
	}

	CComProxyBase::~CComProxyBase()
	{
	}

	void CComProxyBase::frameProc(RCF::ByteBuffer byteBuffer)
	{
	}

	void CComProxyBase::frameProc(unsigned char* byteBuffer, int len)
	{
	}
	
	std::string CComProxyBase::eventProc(std::string strMsg)
	{
		return nullptr;
	}
}