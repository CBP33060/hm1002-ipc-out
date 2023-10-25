#include "com_rpc_client_end_point.h"

namespace maix {
	CComRCFClientEndPoint::CComRCFClientEndPoint(CModule* module)
		: CComClientEndPoint(module)
	{
	}

	mxbool CComRCFClientEndPoint::init(T_COM_PROXY_CLIENT_CONFIG & config,
		E_RCF_CLIENT_TYPE  type)
	{
		m_iClientType = config.m_iType;
		m_type = type;
		if (m_iClientType == COM_PROXY_C_TYPE_SHM)
		{
			m_objSHMClientEndPoint = 
				std::shared_ptr<CComSHMClientEndPoint>(new CComSHMClientEndPoint());
			m_objSHMClientEndPoint->init(config);
		}
		else if (m_iClientType == COM_PROXY_C_TYPE_RTP)
		{
			m_objRTPClientEndPoint =
				std::shared_ptr<CComRTPClientEndPoint>(new CComRTPClientEndPoint());
			m_objRTPClientEndPoint->init(config);
		}
		else
		{
			m_objClientEndPoint = std::shared_ptr<CClientEndPoint>(new CClientEndPoint());
			m_objClientEndPoint->init(config);
		}
		
		return mxtrue;
	}

	std::string CComRCFClientEndPoint::output(std::string strServerName, 
		unsigned char * pcData, int iLen)
	{
		if (m_iClientType == COM_PROXY_C_TYPE_SHM)
		{
			return m_objSHMClientEndPoint->output(pcData, iLen);
		}
		else if (m_iClientType == COM_PROXY_C_TYPE_RTP)
		{
			return m_objRTPClientEndPoint->output(pcData, iLen);
		}
		else
		{
			if (m_type == E_CLIENT_EVENT)
			{
				std::string tmpMsg((char*)pcData, iLen);
				return m_objClientEndPoint->eventProc(tmpMsg);
			}
			else if (m_type == E_CLIENT_FRAME)
			{
				if (m_objClientEndPoint->connect())
				{
					RCF::ByteBuffer  byteBuffer((char*)pcData, iLen);
					m_objClientEndPoint->frameProc(byteBuffer);
				}
				else
				{
					return std::string("disconnect");
				}
			}
		}
		
		return std::string();
	}
	
	int CComRCFClientEndPoint::getClientType()
	{
		return m_iClientType;
	}
}