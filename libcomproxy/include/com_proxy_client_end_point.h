#ifndef __COM_PROXY_CLIENT_END_POINT_H__
#define __COM_PROXY_CLIENT_END_POINT_H__
#include "com_proxy_base.h"
#include <string>

#define COM_PROXY_C_TYPE_TCP		0x01
#define COM_PROXY_C_TYPE_UDP		0x02
#define COM_PROXY_C_TYPE_UNIX		0x04
#define COM_PROXY_C_TYPE_SHM		0x08
#define COM_PROXY_C_TYPE_RTP		0x10

namespace maix {
	typedef struct COM_PROXY_CLIENT_CONFIG
	{
		int			m_iType;
		std::string	m_strIP;
		int			m_iPort;
		std::string	m_strUnix;

		int			m_iTCPMsgLen;
		int			m_iUDPMsgLen;
		int			m_iUNIXMsgLen;

	}T_COM_PROXY_CLIENT_CONFIG;

	class MAIX_EXPORT CClientEndPoint
	{
	public:
		CClientEndPoint();
		virtual ~CClientEndPoint();

		bool init(T_COM_PROXY_CLIENT_CONFIG &config);
		bool connect();
		bool disconnect();
		void frameProc(RCF::ByteBuffer byteBuffer);
		std::string eventProc(std::string strMsg);

	private:
		RCF::RcfInit m_init;
		RcfClient<I_ComProxy> m_client;
	};
}
#endif //__COM_PROXY_CLIENT_END_POINT_H__
