#ifndef __COM_PROXY_SERVER_END_POINT_H__
#define __COM_PROXY_SERVER_END_POINT_H__
#include "com_proxy_base.h"

#define COM_PROXY_S_TYPE_TCP		0x01
#define COM_PROXY_S_TYPE_UDP		0x02
#define COM_PROXY_S_TYPE_UNIX		0x04
#define COM_PROXY_S_TYPE_SHM		0x08
#define COM_PROXY_S_TYPE_RTP		0x10

namespace maix {
	typedef struct COM_PROXY_SERVER_CONFIG
	{
		int			m_iType;
		std::string	m_strIP;
		int			m_iPort;
		std::string	m_strUnix;
		std::string	m_strServerName;
		int			m_iTCPMsgLen;
		int			m_iUDPMsgLen;
		int			m_iUNIXMsgLen;

		int			m_iTCPConLimit;
		int			m_iUDPConLimit;
		int			m_iUNIXConLimit;

	}T_COM_PROXY_SERVER_CONFIG;

	typedef bool(*ComServerAccessProc)(int param);

	class MAIX_EXPORT CServerEndPoint
	{
	public:
		CServerEndPoint();
		virtual ~CServerEndPoint();

		bool init(T_COM_PROXY_SERVER_CONFIG &config, CComProxyBase *obj);
		bool unInit();
		bool enableMultiThread(int min, int max);
		bool accessControlCallBack(ComServerAccessProc *fun);
		bool start();
		bool stop();
	private:
		RCF::RcfInit m_init;
		RCF::RcfServer m_server;
		RCF::ServerBindingPtr m_bindingPtr;

		CComProxyBase *m_objProxyBase;
	};
}
#endif //__COM_PROXY_SERVER_END_POINT_H__