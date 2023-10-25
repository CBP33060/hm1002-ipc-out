#include "com_proxy_udp_server.h"
#include "log_mx.h"

#define XM_UDP_SEND_START		0x0A0A0A0A
#define XM_UDP_SEND_END			0x05050505

#define XM_UDP_RCV_BUF_LEN_MAX (32 * 1024)
namespace maix {
#ifdef _WIN32
	DWORD WINAPI ThreadFun(LPVOID pM)
	{
		SOCKADDR_IN addrClient;
		int len = sizeof(SOCKADDR_IN);
		int nStart = -1;
		int nEnd = -1;
		int nRcvLen = 0;
		int nTotalLen = 0;

		CUDPServer *objUDPServer = (CUDPServer *)pM;
		char *pRcvBuffer = (char*)malloc(XM_UDP_RCV_BUF_LEN_MAX);

		while (objUDPServer->m_bStart && pRcvBuffer)
		{
			nRcvLen = recvfrom(objUDPServer->m_fd, (char*)&nStart,
				sizeof(int), 0,
				(SOCKADDR*)&addrClient, &len);

			if (nRcvLen > 0)
			{
				if (nStart == XM_UDP_SEND_START)
				{
					nRcvLen = recvfrom(objUDPServer->m_fd, (char*)&nTotalLen,
						sizeof(int), 0,
						(SOCKADDR*)&addrClient, &len);
					if (nRcvLen > 0)
					{
						if (nTotalLen > XM_UDP_RCV_BUF_LEN_MAX)
						{
							int nRcvNum = nTotalLen;
							int nRcvOne = XM_UDP_RCV_BUF_LEN_MAX;
							while (nRcvNum)
							{
								memset(pRcvBuffer, 0, XM_UDP_RCV_BUF_LEN_MAX);
								nRcvLen = recvfrom(objUDPServer->m_fd, pRcvBuffer,
									nRcvOne, 0,
									(SOCKADDR*)&addrClient, &len);

								if (nRcvLen == -1)
								{
									int err = WSAGetLastError();
									logPrint(MX_LOG_ERROR, "error!, error code is %d", err);
									break;
								}

								memcpy(objUDPServer->m_ucRecvBuffer + (nTotalLen - nRcvNum),
									pRcvBuffer, nRcvLen);

								nRcvNum -= nRcvLen;

								if (nRcvNum > XM_UDP_RCV_BUF_LEN_MAX)
								{
									nRcvOne = XM_UDP_RCV_BUF_LEN_MAX;
								}
								else
								{
									nRcvOne = nRcvNum;
								}

							}
						}
						else
						{
							nRcvLen = recvfrom(objUDPServer->m_fd, pRcvBuffer,
								nTotalLen, 0,
								(SOCKADDR*)&addrClient, &len);

							memcpy(objUDPServer->m_ucRecvBuffer, pRcvBuffer, nRcvLen);
						}

						nRcvLen = recvfrom(objUDPServer->m_fd, (char*)&nEnd,
							sizeof(int), 0,
							(SOCKADDR*)&addrClient, &len);
						if (nRcvLen > 0)
						{
							if (nEnd == XM_UDP_SEND_END)
							{
								objUDPServer->m_objComProxy->frameProc(objUDPServer->m_ucRecvBuffer, nTotalLen);
							}
						}
					}
				}
			}

		}
		return 0;

	}
#endif

	CUDPServer::CUDPServer()
		:m_iUDPMsgLen(1024)
	{

	}

	CUDPServer::~CUDPServer()
	{
	}

	bool CUDPServer::init(T_COM_PROXY_UDP_S_CONFIG & config, CComProxyBase * obj)
	{
		if (config.m_iUDPMsgLen <= 0)
		{
			m_iUDPMsgLen = 1024;

		}
		else
		{
			m_iUDPMsgLen = config.m_iUDPMsgLen;
		}
		m_ucRecvBuffer = (unsigned char*)malloc(m_iUDPMsgLen);

		if (!m_ucRecvBuffer)
			return false;

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

		SOCKADDR_IN addrSrv;
		addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addrSrv.sin_family = AF_INET;
		addrSrv.sin_port = htons(config.m_iPort);

		bind(m_fd, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		int nRcvBuf = XM_UDP_RCV_BUF_LEN_MAX;
		setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRcvBuf, sizeof(int));

#else
#endif
		m_objComProxy = obj;
		return true;
	}

	bool CUDPServer::unInit()
	{
#ifdef _WIN32
		closesocket(m_fd);
		WSACleanup();
#else
#endif
		return true;
	}

	bool CUDPServer::start()
	{
		m_bStart = true;
#ifdef _WIN32
		HANDLE handle = CreateThread(NULL, 0, ThreadFun, this, 0, NULL);
		//WaitForSingleObject(handle, INFINITE);
#else

#endif
		return true;
	}

	bool CUDPServer::stop()
	{
		m_bStart = false;
		return true;
	}

}