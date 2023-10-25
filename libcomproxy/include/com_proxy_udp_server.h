#ifndef __COM_PROXY_UDP_SERVER_H__
#define __COM_PROXY_UDP_SERVER_H__
#ifdef _WIN32
#include <Winsock2.h>
#else
#endif
#include "com_proxy_base.h"

namespace maix {
	typedef struct COM_PROXY_UDP_S_CONFIG
	{
		std::string	m_strIP;
		int			m_iPort;
		std::string	m_strServerName;
		int			m_iUDPMsgLen;
		int			m_iUDPConLimit;

	}T_COM_PROXY_UDP_S_CONFIG;

	class MAIX_EXPORT CUDPServer
	{
	public:
		CUDPServer();
		virtual ~CUDPServer();

		bool init(T_COM_PROXY_UDP_S_CONFIG &config, CComProxyBase *obj);
		bool unInit();
		bool start();
		bool stop();
#ifdef _WIN32
		SOCKET m_fd;
#else
		int m_fd;

#endif
		unsigned char* m_ucRecvBuffer;
		bool m_bStart;
		int m_iUDPMsgLen;
		CComProxyBase *m_objComProxy;
	};
}
#endif //__COM_PROXY_UDP_SERVER_H__