#ifndef __COM_PROXY_MULTI_END_POINT_H__
#define __COM_PROXY_MULTI_END_POINT_H__
#include "com_proxy_base.h"
#include <string>

namespace maix {
	typedef struct COM_PROXY_MULTI_CONFIG
	{
		std::string	m_strIP;
		int			m_iPort;
		std::string	m_strMutiIP;
		std::string	m_strServerName;
		int			m_iUDPMsgLen;
		int			m_iUDPConLimit;
	}T_COM_PROXY_MULTI_CONFIG;

	typedef bool(*ComServerAccessProc)(int param);

	class MAIX_EXPORT CMultiEndPoint
	{
	public:
		CMultiEndPoint();
		virtual ~CMultiEndPoint();

		bool init(T_COM_PROXY_MULTI_CONFIG &config, class CComProxyBase *obj);
		bool enableMultiThread(int min, int max);
		bool accessControlCallBack(ComServerAccessProc *fun);
		bool start();
		bool stop();

	private:
		RCF::RcfInit m_init;
		RCF::RcfServer m_server;
		RCF::ServerBindingPtr m_bindingPtr;

	};
}
#endif //__COM_PROXY_MULTI_END_POINT_H__
