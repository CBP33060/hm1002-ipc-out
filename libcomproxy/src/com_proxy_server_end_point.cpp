#include "com_proxy_server_end_point.h"
#ifdef _WIN32
#include <RCF/Win32NamedPipeEndpoint.hpp>
#else 
#include <RCF/UnixLocalEndpoint.hpp>
#endif
#include <iostream>
#include "log_mx.h"

namespace maix {
	static bool ProcServiceAccess(int methodId)
	{
		logPrint(MX_LOG_INFOR, "ProcServiceAccess: %d", methodId);
		return true;
	}

	CServerEndPoint::CServerEndPoint()
	{
	}

	CServerEndPoint::~CServerEndPoint()
	{
	}

	bool CServerEndPoint::init(T_COM_PROXY_SERVER_CONFIG & config, CComProxyBase * obj)
	{
		try
		{
			if ((config.m_iType & COM_PROXY_S_TYPE_TCP) == COM_PROXY_S_TYPE_TCP)
			{
				RCF::ServerTransport &tcpTransport =
					m_server.addEndpoint(RCF::TcpEndpoint(config.m_strIP, config.m_iPort));

				if (config.m_iTCPMsgLen > 0)
				{
					tcpTransport.setMaxIncomingMessageLength(config.m_iTCPMsgLen);
				}

				if (config.m_iTCPConLimit)
				{
					tcpTransport.setConnectionLimit(config.m_iTCPConLimit);
				}
			}

			if ((config.m_iType & COM_PROXY_S_TYPE_UDP) == COM_PROXY_S_TYPE_UDP)
			{
				RCF::ServerTransport &udpTransport =
					m_server.addEndpoint(RCF::UdpEndpoint(config.m_strIP, config.m_iPort));

				if (config.m_iUDPMsgLen > 0)
				{
					udpTransport.setMaxIncomingMessageLength(config.m_iUDPMsgLen);
				}

				if (config.m_iUDPConLimit)
				{
					udpTransport.setConnectionLimit(config.m_iUDPConLimit);
				}
			}

			if ((config.m_iType & COM_PROXY_S_TYPE_UNIX) == COM_PROXY_S_TYPE_UNIX)
			{
#ifdef _WIN32
				RCF::ServerTransport & unixTransport =
					m_server.addEndpoint(RCF::Win32NamedPipeEndpoint(config.m_strUnix));
#else 
				unlink(config.m_strUnix.c_str());
				RCF::ServerTransport & unixTransport =
					m_server.addEndpoint(RCF::UnixLocalEndpoint(config.m_strUnix));
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

			m_bindingPtr = m_server.bind<I_ComProxy>(*obj);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}

		m_objProxyBase = obj;
		return true;
	}

	bool CServerEndPoint::unInit()
	{
		return false;
	}

	bool CServerEndPoint::enableMultiThread(int min, int max)
	{
		if (min <= 0 || max <= 0 || min > max)
		{
			return false;
		}

		try
		{
			RCF::ThreadPoolPtr threadPoolPtr(new RCF::ThreadPool(min, max));
			m_server.setThreadPool(threadPoolPtr);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CServerEndPoint::accessControlCallBack(ComServerAccessProc * fun)
	{
		if (!m_bindingPtr)
			return false;
		try
		{
			auto accessControl = [&](int methodId) { return ProcServiceAccess(methodId); };
			m_bindingPtr->setAccessControl(accessControl);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}

		return true;
	}

	bool CServerEndPoint::start()
	{
		try
		{
			m_server.start();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CServerEndPoint::stop()
	{
		try
		{
			m_server.stop();
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}
}