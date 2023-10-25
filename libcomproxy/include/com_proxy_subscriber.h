#ifndef __COM_PROXY_SUBSCRIBER_H__
#define __COM_PROXY_SUBSCRIBER_H__
#include "global_export.h"
#include <RCF/RCF.hpp>
#include "com_proxy_base.h"

#define COM_PROXY_SB_TYPE_TCP		0x01
#define COM_PROXY_SB_TYPE_UDP		0x02
#define COM_PROXY_SB_TYPE_UNIX		0x04

namespace maix {
	typedef struct COM_PROXY_SUB_CONFIG
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

	}T_COM_PROXY_SUB_CONFIG;

	class MAIX_EXPORT CSubscriber
	{
	public:
		CSubscriber();
		virtual ~CSubscriber();

		bool init(T_COM_PROXY_SUB_CONFIG &config);
		bool subAdd(T_COM_PROXY_SUB_CONFIG &config, class CComProxyBase *obj);
		bool enableMultiThread(int min, int max);
		bool start();
		bool stop();

	private:
		RCF::RcfInit m_init;
		RCF::RcfServer m_subServer;
		RCF::SubscriptionPtr m_subscriptionPtr;
	};
}
#endif //__XM_COM_PROXY_SUBSCRIBER_HPP__
