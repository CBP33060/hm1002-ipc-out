#include "com_proxy_multi_end_point.h"
#include <iostream>
#include "log_mx.h"

namespace maix {
	static bool ProcServiceAccess(int methodId)
	{
		logPrint(MX_LOG_INFOR, "ProcServiceAccess: %d", methodId);
		return true;
	}

	CMultiEndPoint::CMultiEndPoint()
	{
	}

	CMultiEndPoint::~CMultiEndPoint()
	{
	}

	bool CMultiEndPoint::init(T_COM_PROXY_MULTI_CONFIG & config, CComProxyBase * obj)
	{
		try
		{
			RCF::UdpEndpoint udpEndpoint(config.m_strIP, config.m_iPort);
			udpEndpoint.listenOnMulticast(config.m_strMutiIP);
			RCF::ServerTransport &udpTransport = m_server.addEndpoint(udpEndpoint);

			if (config.m_iUDPMsgLen > 0)
			{
				udpTransport.setMaxIncomingMessageLength(config.m_iUDPMsgLen);
			}

			if (config.m_iUDPConLimit)
			{
				udpTransport.setConnectionLimit(config.m_iUDPConLimit);
			}

			m_bindingPtr = m_server.bind<I_ComProxy>(*obj);
		}
		catch (const RCF::Exception & e)
		{
			logPrint(MX_LOG_ERROR, "Error: %s", e.getErrorMessage().c_str());
			return false;
		}
		return true;
	}

	bool CMultiEndPoint::enableMultiThread(int min, int max)
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

	bool CMultiEndPoint::accessControlCallBack(ComServerAccessProc * fun)
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

	bool CMultiEndPoint::start()
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

	bool CMultiEndPoint::stop()
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