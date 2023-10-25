#include "com_rtp_server_end_point.h"

namespace maix {
#ifdef _WIN32
	DWORD WINAPI RTPThreadFun(LPVOID arg)
#else
	static void * RTPThreadFun(void *arg)
#endif
	{
		CComRTPSeverEndPoint *objComRTPSeverEndPoint =
			(CComRTPSeverEndPoint *)arg;

		if (objComRTPSeverEndPoint == NULL)
#ifdef _WIN32
			return -1;
#else
			return NULL;
#endif

		CComProxyBase *objComProxyHandle =
			objComRTPSeverEndPoint->m_comProxyHandle;

		while (objComRTPSeverEndPoint->m_bRun)
		{
#ifndef RTP_SUPPORT_THREAD
			objComRTPSeverEndPoint->m_rtpSession.Poll();
#endif // RTP_SUPPORT_THREAD

			objComRTPSeverEndPoint->m_rtpSession.BeginDataAccess();

			if (objComRTPSeverEndPoint->m_rtpSession.GotoFirstSourceWithData())
			{
				do
				{
					RTPPacket *pack;
					while ((pack = objComRTPSeverEndPoint->m_rtpSession.GetNextPacket()) != NULL)
					{
						if (objComProxyHandle)
						{
							bool bRet = objComRTPSeverEndPoint->addDataPacket(
								(unsigned char*)pack->GetPayloadData(),
								pack->GetPayloadLength());

							if (bRet == false)
							{
								objComRTPSeverEndPoint->m_iDataIndex = 0;
							}
							else
							{
								bool hasmarker = pack->HasMarker();
								if (hasmarker)
								{
									objComProxyHandle->frameProc(
										objComRTPSeverEndPoint->m_ucDataBuffer,
										objComRTPSeverEndPoint->m_iDataIndex);

									objComRTPSeverEndPoint->m_iDataIndex = 0;
								}
								else
								{
									//printf("rcv: %d\n", objComRTPSeverEndPoint->m_iDataIndex);
								}
							}
						}
						
						objComRTPSeverEndPoint->m_rtpSession.DeletePacket(pack);
					}
				} while (objComRTPSeverEndPoint->m_rtpSession.GotoNextSourceWithData());
			}
			else
			{
				RTPTime::Wait(RTPTime(0, 100));
			}

			objComRTPSeverEndPoint->m_rtpSession.EndDataAccess();

		}
		return 0;

	}

	CComRTPSeverEndPoint::CComRTPSeverEndPoint()
	{
		m_ucDataBuffer = NULL;
		m_bRun = false;
		m_comProxyHandle = NULL;
		m_iDataBufferLen = 0;
		m_iDataIndex = 0;
	}

	CComRTPSeverEndPoint::~CComRTPSeverEndPoint()
	{
	}

	bool CComRTPSeverEndPoint::init(T_COM_PROXY_SERVER_CONFIG & config, 
		CComProxyBase * handle)
	{
		if (handle == NULL)
			return false;

		if (m_ucDataBuffer == NULL)
		{
			m_ucDataBuffer = (unsigned char*)malloc(config.m_iUDPMsgLen);
			memset(m_ucDataBuffer, 0, config.m_iUDPMsgLen);
		}

		m_iDataBufferLen = config.m_iUDPMsgLen;

		m_comProxyHandle = handle;

		int ret = 0;
		m_rtpSessparams.SetOwnTimestampUnit(1.0 / 10.0);
		m_rtpSessparams.SetAcceptOwnPackets(true);
		m_rtpSessparams.SetMaximumPacketSize(RTP_SERVER_DATA_LEN + 64);
		m_rtpSessparams.SetUsePollThread(true);
		m_rtpTransparams.SetPortbase(config.m_iPort);
		ret = m_rtpSession.Create(m_rtpSessparams, &m_rtpTransparams);
		if (ret != 0)
		{
			return false;
		}

		return true;
	}

	bool CComRTPSeverEndPoint::start()
	{
		m_bRun = true;
#ifdef _WIN32
		m_threadHandle = CreateThread(NULL, 0, RTPThreadFun, this, 0, NULL);

		if (m_threadHandle == NULL)
		{
			return false;
		}

#else
		int ret = 0;
		ret = pthread_create(&m_threadHandle, NULL, RTPThreadFun, this);
		if (ret != 0)
		{
			return false;
		}
#endif
		return true;
	}

	bool CComRTPSeverEndPoint::stop()
	{
		return true;
	}

	bool CComRTPSeverEndPoint::addDataPacket(unsigned char * ucData, int iLen)
	{
		if (ucData == NULL || iLen == 0)
			return false;

		if ((m_iDataIndex + iLen) > m_iDataBufferLen)
			return false;

		memcpy(m_ucDataBuffer + m_iDataIndex, ucData, iLen);
		m_iDataIndex += iLen;
		return true;
	}
}
