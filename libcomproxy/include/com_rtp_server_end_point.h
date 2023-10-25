#ifndef __COM_RTP_SERVER_END_POINT_H__
#define __COM_RTP_SERVER_END_POINT_H__
#include "com_proxy_server_end_point.h"
#include "rtp/rtpsession.h"
#include "rtp/rtpudpv4transmitter.h"
#include "rtp/rtpipv4address.h"
#include "rtp/rtpsessionparams.h"
#include "rtp/rtperrors.h"
#include "rtp/rtppacket.h"
#define RTP_SERVER_DATA_LEN (32*1024)           // < 64

using namespace jrtplib;
namespace maix {
	class MAIX_EXPORT CComRTPSeverEndPoint
	{
	public:
		CComRTPSeverEndPoint();
		virtual ~CComRTPSeverEndPoint();

		bool init(T_COM_PROXY_SERVER_CONFIG &config, CComProxyBase * handle);
		bool start();
		bool stop();

		bool addDataPacket(unsigned char *ucData, int iLen);
	public:
		bool m_bRun;
		RTPSession m_rtpSession;
		CComProxyBase *m_comProxyHandle;
		unsigned char *m_ucDataBuffer;
		int m_iDataBufferLen;
		int m_iDataIndex;
	private:
		
		RTPUDPv4TransmissionParams m_rtpTransparams;
		RTPSessionParams m_rtpSessparams;
		RTPIPv4Address m_rtpAddr;
#ifdef _WIN32
		HANDLE m_threadHandle;
#else
		pthread_t m_threadHandle;
#endif
	};
}
#endif //__COM_RTP_SERVER_END_POINT_H__
