#include "com_rpc_server_end_point.h"

namespace maix {
	CComRcfServerEndPoint::CComRcfServerEndPoint(CModule* module)
		:CComServerEndPoint(module)
	{
	}

	CComRcfServerEndPoint::~CComRcfServerEndPoint()
	{
		m_objServerEndPoint->stop();
	}

	mxbool CComRcfServerEndPoint::init(T_COM_PROXY_SERVER_CONFIG &config,
		CComProxyBase * handle)
	{
		if (config.m_iType == COM_PROXY_S_TYPE_SHM)
		{
			m_objSHMSeverEndPoint =
				std::shared_ptr<CComSHMSeverEndPoint>(new CComSHMSeverEndPoint());
			if (m_objSHMSeverEndPoint != NULL)
			{
				if (m_objSHMSeverEndPoint->init(config, handle))
				{
					m_objSHMSeverEndPoint->start();
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				return mxfalse;
			}	
		}
		else if (config.m_iType == COM_PROXY_S_TYPE_RTP)
		{
			m_objRTPSeverEndPoint =
				std::shared_ptr<CComRTPSeverEndPoint>(new CComRTPSeverEndPoint());
			if (m_objRTPSeverEndPoint != NULL)
			{
				if (m_objRTPSeverEndPoint->init(config, handle))
				{
					m_objRTPSeverEndPoint->start();
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				return mxfalse;
			}
		}
		else
		{
			m_objServerEndPoint =
				std::shared_ptr<CServerEndPoint>(new CServerEndPoint());
			m_objServerEndPoint->init(config, handle);
			m_objServerEndPoint->start();
		}

		return mxtrue;
	}

	std::string CComRcfServerEndPoint::input(unsigned char * pcData, int iLen)
	{
		return std::string("error");
	}

	mxbool CComRcfServerEndPoint::accessControlCallBack(ComServerAccessProc * fun)
	{
		return m_objServerEndPoint->accessControlCallBack(fun);
	}
}
