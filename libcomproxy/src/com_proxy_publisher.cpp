#include "com_proxy_publisher.h"
#ifdef _WIN32
#include <RCF/Win32NamedPipeEndpoint.hpp>
#else
#include <RCF/UnixLocalEndpoint.hpp>
#endif
#include <iostream>
#include "log_mx.h"

namespace maix {
	static bool ProcSubscriberConnect(RCF::RcfSession & session, 
		const std::string & topicName)
	{
		logPrint(MX_LOG_INFOR, "ProcSubscriberConnect: %s", 
			topicName.c_str());
		return true;
	}

	static void ProcSubscriberDisconnect(RCF::RcfSession & session, 
		const std::string  & topicName)
	{
		logPrint(MX_LOG_INFOR, "ProcSubscriberDisconnect: %s", 
			topicName.c_str());
	}

	CPublisher::CPublisher()
	{
	}

	CPublisher::~CPublisher()
	{
	}

	bool CPublisher::init(T_COM_PROXY_PUB_CONFIG & config)
	{
		try
		{
			if ((config.m_iType & COM_PROXY_P_TYPE_TCP) == COM_PROXY_P_TYPE_TCP)
			{
				RCF::ServerTransport &tcpTransport =
					m_pubServer.addEndpoint(RCF::TcpEndpoint(config.m_strIP, config.m_iPort));

				if (config.m_iTCPMsgLen > 0)
				{
					tcpTransport.setMaxIncomingMessageLength(config.m_iTCPMsgLen);
				}

				if (config.m_iTCPConLimit)
				{
					tcpTransport.setConnectionLimit(config.m_iTCPConLimit);
				}
			}

			if ((config.m_iType & COM_PROXY_P_TYPE_UDP) == COM_PROXY_P_TYPE_UDP)
			{
				RCF::ServerTransport &udpTransport =
					m_pubServer.addEndpoint(RCF::UdpEndpoint(config.m_strIP, config.m_iPort));

				if (config.m_iUDPMsgLen > 0)
				{
					udpTransport.setMaxIncomingMessageLength(config.m_iUDPMsgLen);
				}

				if (config.m_iUDPConLimit)
				{
					udpTransport.setConnectionLimit(config.m_iUDPConLimit);
				}
			}

			if ((config.m_iType & COM_PROXY_P_TYPE_UNIX) == COM_PROXY_P_TYPE_UNIX)
			{
#ifdef _WIN32
				RCF::ServerTransport & unixTransport =
					m_pubServer.addEndpoint(RCF::Win32NamedPipeEndpoint(config.m_strUnix));
#else
				unlink(config.m_strUnix.c_str());
				RCF::ServerTransport & unixTransport =
					m_pubServer.addEndpoint(RCF::UnixLocalEndpoint(config.m_strUnix));
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

	bool CPublisher::pubConnectFun()
	{
		try
		{
			RCF::PublisherParms pubParms;
			pubParms.setOnSubscriberConnect(ProcSubscriberConnect);
			pubParms.setOnSubscriberDisconnect(ProcSubscriberDisconnect);

			m_publisherPtr =
				m_pubServer.createPublisher<I_ComProxy>(pubParms);

		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}

		return true;
	}

	bool CPublisher::accessControlCallBack(ComPublisherAccessProc * fun)
	{
		return false;
	}

	bool CPublisher::enableMultiThread(int min, int max)
	{
		if (min <= 0 || max <= 0 || min > max)
		{
			return false;
		}

		try
		{
			RCF::ThreadPoolPtr threadPoolPtr(new RCF::ThreadPool(min, max));
			m_pubServer.setThreadPool(threadPoolPtr);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CPublisher::publishMessage(std::string strMsg)
	{
		try
		{
			m_publisherPtr->publish().eventProc(strMsg);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CPublisher::start()
	{
		try
		{
			m_pubServer.start();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CPublisher::stop()
	{
		try
		{
			m_pubServer.stop();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

}