#ifndef __COM_PROXY_UDP_CLIENT_H__
#define __COM_PROXY_UDP_CLIENT_H__
#ifdef _WIN32
#include <Winsock2.h>
#else
#endif
#include "global_export.h"
#include <string>

namespace maix {
	typedef struct COM_PROXY_UDP_C_CONFIG
	{
		std::string	m_strIP;
		int			m_iPort;
		std::string	m_strClientName;
		int			m_iUDPMsgLen;
		int			m_iUDPConLimit;

	}T_COM_PROXY_C_CONFIG;

	class MAIX_EXPORT CUDPClient
	{
	public:
		CUDPClient();
		virtual ~CUDPClient();

		bool init(T_COM_PROXY_C_CONFIG &config);
		void frameProc(unsigned char* byteBuffer, int len);
		bool unInit();

#ifdef _WIN32
		SOCKET m_fd;
		SOCKADDR_IN m_addrSrv;
#else
		int m_fd;

#endif
		bool m_bConnect;
		int m_iUDPMsgLen;
	};
}
#endif //__COM_PROXY_UDP_CLIENT_H__
