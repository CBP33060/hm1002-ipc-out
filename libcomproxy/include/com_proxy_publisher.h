#ifndef __COM_PROXY_PUBLISHER_H__
#define __COM_PROXY_PUBLISHER_H__
#include "global_export.h"
#include <RCF/RCF.hpp>
#include "com_proxy_base.h"

#define COM_PROXY_P_TYPE_TCP		0x01
#define COM_PROXY_P_TYPE_UDP		0x02
#define COM_PROXY_P_TYPE_UNIX		0x04

namespace maix {
	typedef struct COM_PROXY_PUB_CONFIG
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

	}T_COM_PROXY_PUB_CONFIG;

	typedef bool(*ComPublisherAccessProc)(int param);
	typedef RCF::Publisher<I_ComProxy> ComProxyServicePublisher;
	typedef std::shared_ptr< ComProxyServicePublisher > 
		ComProxyServicePublisherPtr;

	class MAIX_EXPORT CPublisher
	{
	public:
		CPublisher();
		virtual ~CPublisher();

		bool init(T_COM_PROXY_PUB_CONFIG &config);
		bool pubConnectFun();
		bool accessControlCallBack(ComPublisherAccessProc *fun);
		bool enableMultiThread(int min, int max);
		bool publishMessage(std::string strMsg);
		bool start();
		bool stop();

	private:
		RCF::RcfInit m_init;
		RCF::RcfServer m_pubServer;
		ComProxyServicePublisherPtr m_publisherPtr;
	};
}
#endif //__COM_PROXY_PUBLISHER_H__
