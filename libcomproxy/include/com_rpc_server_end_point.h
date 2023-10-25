#ifndef __COM_RCF_SERVER_END_POINT_H__
#define __COM_RCF_SERVER_END_POINT_H__
#include "com_server_end_point.h"
#include "com_proxy_server_end_point.h"
#include "com_shm_server_end_point.h"
#include "com_rtp_server_end_point.h"

namespace maix {
	class MAIX_EXPORT CComRcfServerEndPoint : public CComServerEndPoint
	{
	public:
		CComRcfServerEndPoint(CModule* module);
		~CComRcfServerEndPoint();

		mxbool init(T_COM_PROXY_SERVER_CONFIG &config, CComProxyBase * handle);
		virtual std::string  input(unsigned char * pcData, int iLen);
		mxbool accessControlCallBack(ComServerAccessProc *fun);
	private:
		CModule* m_module;
		std::shared_ptr<CServerEndPoint> m_objServerEndPoint;
		std::shared_ptr<CComSHMSeverEndPoint> m_objSHMSeverEndPoint;
		std::shared_ptr<CComRTPSeverEndPoint> m_objRTPSeverEndPoint;
	};
}
#endif //__COM_RCF_SERVER_END_POINT_H__
