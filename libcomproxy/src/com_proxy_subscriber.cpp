#include "com_proxy_subscriber.h"
#ifdef _WIN32
#include <RCF/Win32NamedPipeEndpoint.hpp>
#else
#include <RCF/UnixLocalEndpoint.hpp>
#endif
#include <iostream>
#include "log_mx.h"

namespace maix {
	CSubscriber::CSubscriber()
	{
	}

	CSubscriber::~CSubscriber()
	{
	}

	bool CSubscriber::init(T_COM_PROXY_SUB_CONFIG & config)
	{
		try
		{
			if ((config.m_iType & COM_PROXY_SB_TYPE_TCP) == COM_PROXY_SB_TYPE_TCP)
			{
				RCF::ServerTransport &tcpTransport =
					m_subServer.addEndpoint(RCF::TcpEndpoint(config.m_strIP, config.m_iPort));

				if (config.m_iTCPMsgLen > 0)
				{
					tcpTransport.setMaxIncomingMessageLength(config.m_iTCPMsgLen);
				}

				if (config.m_iTCPConLimit)
				{
					tcpTransport.setConnectionLimit(config.m_iTCPConLimit);
				}
			}

			if ((config.m_iType & COM_PROXY_SB_TYPE_UDP) == COM_PROXY_SB_TYPE_UDP)
			{
				RCF::ServerTransport &udpTransport =
					m_subServer.addEndpoint(RCF::UdpEndpoint(config.m_strIP, 
						config.m_iPort));

				if (config.m_iUDPMsgLen > 0)
				{
					udpTransport.setMaxIncomingMessageLength(config.m_iUDPMsgLen);
				}

				if (config.m_iUDPConLimit)
				{
					udpTransport.setConnectionLimit(config.m_iUDPConLimit);
				}
			}

			if ((config.m_iType & COM_PROXY_SB_TYPE_UNIX) == 
				COM_PROXY_SB_TYPE_UNIX)
			{
#ifdef _WIN32
				RCF::ServerTransport & unixTransport =
					m_subServer.addEndpoint(RCF::Win32NamedPipeEndpoint(config.m_strUnix));
#else	
				unlink(config.m_strUnix.c_str());
				RCF::ServerTransport & unixTransport =
					m_subServer.addEndpoint(RCF::UnixLocalEndpoint(
						config.m_strUnix));
#endif
				if (config.m_iUNIXMsgLen > 0)
				{
					unixTransport.setMaxIncomingMessageLength(config.m_iUNIXMsgLen);
				}

				if (config.m_iUNIXConLimit)
				{
					unixTransport.setConnectionLimit(config.m_iUNIXConLimit);
				}
			}
			else
			{
				return false;
			}

		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CSubscriber::subAdd(T_COM_PROXY_SUB_CONFIG & config, CComProxyBase * obj)
	{
		try
		{
			RCF::SubscriptionParms subParms;

			if (config.m_iType == COM_PROXY_SB_TYPE_TCP)
			{
				subParms.setPublisherEndpoint(RCF::TcpEndpoint(config.m_strIP, 
					config.m_iPort));
			}
			else if (config.m_iType == COM_PROXY_SB_TYPE_UDP)
			{
				subParms.setPublisherEndpoint(RCF::UdpEndpoint(config.m_strIP, 
					config.m_iPort));
			}
			else if (config.m_iType == COM_PROXY_SB_TYPE_UNIX)
			{
#ifdef _WIN32
				subParms.setPublisherEndpoint(RCF::Win32NamedPipeEndpoint(
					config.m_strUnix));
#else
				subParms.setPublisherEndpoint(RCF::UnixLocalEndpoint(
						config.m_strUnix));
#endif
			}
			else
			{
				return false;
			}
			m_subscriptionPtr =
				m_subServer.createSubscription<I_ComProxy>(*obj, subParms);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}

		return true;
	}

	bool CSubscriber::enableMultiThread(int min, int max)
	{
		if (min <= 0 || max <= 0 || min > max)
		{
			return false;
		}

		try
		{
			RCF::ThreadPoolPtr threadPoolPtr(new RCF::ThreadPool(min, max));
			m_subServer.setThreadPool(threadPoolPtr);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CSubscriber::start()
	{
		try
		{
			m_subServer.start();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CSubscriber::stop()
	{
		try
		{
			m_subServer.stop();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

}