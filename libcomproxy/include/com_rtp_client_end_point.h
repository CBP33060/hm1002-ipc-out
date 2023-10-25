#ifndef __COM_RTP_CLIENT_END_POINT_H__
#define __COM_RTP_CLIENT_END_POINT_H__
#include "com_proxy_client_end_point.h"
#include "rtp/rtpsession.h"
#include "rtp/rtpudpv4transmitter.h"
#include "rtp/rtptcpaddress.h"
#include "rtp/rtpsessionparams.h"
#include "rtp/rtperrors.h"
#include "rtp/rtppacket.h"
#define RTP_CLIENT_DATA_LEN (32*1024)       //< 64

using namespace jrtplib;

namespace maix {
	class MAIX_EXPORT CComRTPClientEndPoint
	{
	public:
		CComRTPClientEndPoint();
		virtual ~CComRTPClientEndPoint();
		bool init(T_COM_PROXY_CLIENT_CONFIG &config);
		virtual std::string  output(unsigned char * pcData, int iLen);
		int getClientType();

	private:
		int m_iClientType;
		int m_bTryNum;
		RTPSession m_rtpSession;
		RTPUDPv4TransmissionParams m_rtpTransparams;
		RTPSessionParams m_rtpSessparams;
	};
}
#endif //__COM_PROXY_UDP_CLIENT_H__
