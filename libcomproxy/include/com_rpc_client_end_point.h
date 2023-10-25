#ifndef __COM_RCF_CLIENT_END_POINT_H__
#define __COM_RCF_CLIENT_END_POINT_H__
#include "com_client_end_point.h"
#include "com_proxy_client_end_point.h"
#include "com_shm_client_end_point.h"
#include "com_rtp_client_end_point.h"

namespace maix {
	enum E_RCF_CLIENT_TYPE
	{
		E_CLIENT_EVENT,
		E_CLIENT_FRAME,
	};
	class MAIX_EXPORT CComRCFClientEndPoint : public CComClientEndPoint
	{
	public:
		CComRCFClientEndPoint(CModule* module);
		mxbool init(T_COM_PROXY_CLIENT_CONFIG &config, E_RCF_CLIENT_TYPE type);
		virtual std::string  output(std::string strServerName,
			unsigned char * pcData, int iLen);
		int getClientType();
	private:
		CModule* m_module;
		int m_iClientType;
		E_RCF_CLIENT_TYPE m_type;
		std::shared_ptr<CClientEndPoint> m_objClientEndPoint;
		std::shared_ptr<CComSHMClientEndPoint> m_objSHMClientEndPoint;
		std::shared_ptr<CComRTPClientEndPoint> m_objRTPClientEndPoint;
	};
}
#endif //__COM_RCF_CLIENT_END_POINT_H__
