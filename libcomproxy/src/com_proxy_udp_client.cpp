#include "com_proxy_udp_client.h"
#ifdef _WIN32
#include <Winsock2.h>
#endif
#include "log_mx.h"

#define XM_UDP_SEND_START		0x0A0A0A0A
#define XM_UDP_SEND_END			0x05050505

#define XM_UDP_SEND_BUF_LEN_MAX (32 * 1024)
namespace maix {
	CUDPClient::CUDPClient()
	{
	}

	CUDPClient::~CUDPClient()
	{
	}

	bool CUDPClient::init(T_COM_PROXY_C_CONFIG & config)
	{
		if (config.m_iUDPMsgLen <= 0)
		{
			m_iUDPMsgLen = 1024;

		}
		else
		{
			m_iUDPMsgLen = config.m_iUDPMsgLen;
		}

#ifdef _WIN32
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		if (err != 0)
		{
			return false;
		}

		if (LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 2)
		{
			WSACleanup();
			return false;
		}

		m_fd = socket(AF_INET, SOCK_DGRAM, 0);
		m_addrSrv.sin_addr.s_addr = inet_addr(config.m_strIP.c_str());
		m_addrSrv.sin_family = AF_INET;
		m_addrSrv.sin_port = htons(config.m_iPort);

		int nSendBuf = XM_UDP_SEND_BUF_LEN_MAX;
		setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
#else
#endif
		return true;
	}

	void CUDPClient::frameProc(unsigned char * byteBuffer, int len)
	{
#ifdef _WIN32
		int ret = -1;
		int nSendOne = 0;
		int nSendTotal = 0;
		int nStart = XM_UDP_SEND_START;
		int nEnd = XM_UDP_SEND_END;
		
		ret = sendto(m_fd, (char*)&nStart, sizeof(int), 0,
			(SOCKADDR*)&m_addrSrv, sizeof(SOCKADDR));

		if (ret == -1)
		{
			int err = WSAGetLastError();
			logPrint(MX_LOG_ERROR, "error!, error code is %d", err);
			return;
		}

		ret = sendto(m_fd, (char*)&len, sizeof(int), 0,
			(SOCKADDR*)&m_addrSrv, sizeof(SOCKADDR));

		if (ret == -1)
		{
			int err = WSAGetLastError();
			logPrint(MX_LOG_ERROR, "error!, error code is %d", err);
			return;
		}

		if (len > XM_UDP_SEND_BUF_LEN_MAX)
		{
			nSendOne = XM_UDP_SEND_BUF_LEN_MAX;
		}

		while (1)
		{
			ret = sendto(m_fd, (char*)byteBuffer + nSendTotal, nSendOne, 0,
				(SOCKADDR*)&m_addrSrv, sizeof(SOCKADDR));
			nSendTotal += nSendOne;
			if ((len - nSendTotal) > XM_UDP_SEND_BUF_LEN_MAX)
			{
				nSendOne = XM_UDP_SEND_BUF_LEN_MAX;
			}
			else
			{
				nSendOne = len - nSendTotal;
			}

			if (ret == -1 || nSendTotal == len)
				break;
		}
		if (ret == -1)
		{
			int err = WSAGetLastError();
			logPrint(MX_LOG_ERROR, "error!, error code is %d", err);
			return;
		}

		ret = sendto(m_fd, (char*)&nEnd, sizeof(int), 0,
			(SOCKADDR*)&m_addrSrv, sizeof(SOCKADDR));
#else
#endif
	}

	bool CUDPClient::unInit()
	{
#ifdef _WIN32
		closesocket(m_fd);
		WSACleanup();
#else
#endif
		return true;
	}
}