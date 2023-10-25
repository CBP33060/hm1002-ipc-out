#include "com_proxy_client_end_point.h"
#ifdef _WIN32
#include <RCF/Win32NamedPipeEndpoint.hpp>
#else
#include <RCF/UnixLocalEndpoint.hpp>
#endif
#include <iostream>
#include "log_mx.h"

namespace maix {
	CClientEndPoint::CClientEndPoint()
	{
	}

	CClientEndPoint::~CClientEndPoint()
	{
		disconnect();
	}

	bool CClientEndPoint::init(T_COM_PROXY_CLIENT_CONFIG & config)
	{
		try
		{
			if (config.m_iType == COM_PROXY_C_TYPE_TCP)
			{
				RcfClient<I_ComProxy> client(
					(RCF::TcpEndpoint(config.m_strIP, config.m_iPort)));
				if (config.m_iTCPMsgLen > 0)
				{
					client.getClientStub().getTransport().
						setMaxIncomingMessageLength(config.m_iTCPMsgLen);
				}

				m_client = client;
			}
			else if (config.m_iType == COM_PROXY_C_TYPE_UDP)
			{
				RcfClient<I_ComProxy> client(
					(RCF::UdpEndpoint(config.m_strIP, config.m_iPort)));
				if (config.m_iUDPMsgLen > 0)
				{
					client.getClientStub().getTransport().
						setMaxIncomingMessageLength(config.m_iUDPMsgLen);
				}
				m_client = client;
			}
			else if (config.m_iType == COM_PROXY_C_TYPE_UNIX)
			{
#ifdef _WIN32
				RcfClient<I_ComProxy> client(
					(RCF::Win32NamedPipeEndpoint(config.m_strUnix)));

				if (config.m_iUNIXMsgLen > 0)
				{
					client.getClientStub().getTransport().
						setMaxIncomingMessageLength(config.m_iUNIXMsgLen);
				}
				m_client = client;

#else
				RcfClient<I_ComProxy> client((RCF::UnixLocalEndpoint(config.m_strUnix)));

				if (config.m_iUNIXMsgLen > 0)
				{
					client.getClientStub().getTransport().
					setMaxIncomingMessageLength(config.m_iUNIXMsgLen);
				}
				
				m_client = client;
#endif
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

	bool CClientEndPoint::connect()
	{
		bool ret = false;
		try
		{
			ret = m_client.getClientStub().isConnected();
			if (ret == false)
			{
				m_client.getClientStub().connect();
				ret = m_client.getClientStub().isConnected();
			}
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_DEBUG, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return ret;
	}

	bool CClientEndPoint::disconnect()
	{
		bool ret = false;
		try
		{
			m_client.getClientStub().disconnect();
			ret = m_client.getClientStub().isConnected();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_DEBUG, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return ret;
	}

	void CClientEndPoint::frameProc(RCF::ByteBuffer byteBuffer)
	{
		try
		{
			m_client.frameProc(byteBuffer);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_DEBUG, "Error: %s", e.getErrorMessage().c_str());
		}
	}

	std::string CClientEndPoint::eventProc(std::string strMsg)
	{
		std::string ret;
		try
		{
			ret = m_client.eventProc(strMsg);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_DEBUG, "Error: %s", e.getErrorMessage().c_str());
		}
		return ret;
	}
}