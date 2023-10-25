#include "com_rtp_client_end_point.h"

namespace maix {
	CComRTPClientEndPoint::CComRTPClientEndPoint()
	{
		m_bTryNum = 0;
	}

	CComRTPClientEndPoint::~CComRTPClientEndPoint()
	{
		m_rtpSession.ClearDestinations();
		m_rtpSession.BYEDestroy(RTPTime(0, 100), 0, 0);
		m_rtpSession.Destroy();
	}

	bool CComRTPClientEndPoint::init(T_COM_PROXY_CLIENT_CONFIG & config)
	{
		int ret = -1;
		uint32_t uiDestIP;

		uiDestIP = inet_addr(config.m_strIP.c_str());
		uiDestIP = ntohl(uiDestIP);

		m_rtpSessparams.SetOwnTimestampUnit(1.0 / 10.0);
		m_rtpSessparams.SetMaximumPacketSize(RTP_CLIENT_DATA_LEN + 64);
		m_rtpTransparams.SetPortbase(config.m_iPort + 200);
		ret = m_rtpSession.Create(m_rtpSessparams, &m_rtpTransparams);
		if (ret != 0)
		{
			return false;
		}

		m_rtpSession.SetDefaultMark(true);
		RTPIPv4Address addr(uiDestIP, config.m_iPort);
		ret = m_rtpSession.AddDestination(addr);
		if (ret != 0)
		{
			return false;
		}

		return true;
	}

	std::string CComRTPClientEndPoint::output(unsigned char * pcData, int iLen)
	{
		int ret = 0;
		int iTmpLen = iLen;
		int iSendLen = 0;
		while (iTmpLen)
		{
			if (iTmpLen > RTP_CLIENT_DATA_LEN)
			{
				ret = m_rtpSession.SendPacket((void *)(pcData + iSendLen), 
					RTP_CLIENT_DATA_LEN, 0, false, 10);
				if (ret != 0)
				{
					m_bTryNum++;
					if (m_bTryNum == 5)
					{
						return std::string("disconnect");
					}
					printf("send out ret: %d\n", ret);
				}
				else
				{
					m_bTryNum = 0;
				}

				iTmpLen -= RTP_CLIENT_DATA_LEN;
				iSendLen += RTP_CLIENT_DATA_LEN;
			}
			else
			{
				ret = m_rtpSession.SendPacket((void *)(pcData + iSendLen), 
					iTmpLen, 0, true, 10);
				if (ret != 0)
				{
					m_bTryNum++;
					if (m_bTryNum == 5)
					{
						return std::string("disconnect");
					}
					printf("send out ret: %d\n", ret);
				}
				else
				{
					m_bTryNum = 0;
				}

				break;
			}
		}
		return std::string();
	}

	int CComRTPClientEndPoint::getClientType()
	{
		return 0;
	}
}
