#include "event_manage_attempt_report.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

namespace maix {
	CEventManageAttemptReport::CEventManageAttemptReport(CModule * module)
		: m_module(module)
		, m_objEventManageSendFailedQueue(NULL)
	{
	}

	CEventManageAttemptReport::~CEventManageAttemptReport()
	{
	}

	mxbool CEventManageAttemptReport::init()
	{
		if (m_module == NULL)
			return mxfalse;

		if (!m_tEventMessageQueue.init(10, 0))
			return mxfalse;

		std::shared_ptr<CEventManageSendFailedQueue> objEventManageSendFailedQueue =
			std::make_shared<CEventManageSendFailedQueue>(this);

		if (objEventManageSendFailedQueue == NULL)
			return mxfalse;

		if (!objEventManageSendFailedQueue->init())
			return mxfalse;

		if (!m_module->getConfig("IPC_MANAGE_REMOTE_EVENT", 
			"GUID", m_strIPCManageGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("IPC_MANAGE_REMOTE_EVENT", 
			"SERVER", m_strIPCManageEventServer))
		{
			return mxfalse;
		}

		m_threadEventManageFailedQueue = std::thread([objEventManageSendFailedQueue]() {
			objEventManageSendFailedQueue->run();
		});

		m_objEventManageSendFailedQueue = objEventManageSendFailedQueue;
		return mxtrue;
	}

	mxbool CEventManageAttemptReport::unInit()
	{
		return mxtrue;
	}

	void CEventManageAttemptReport::run()
	{
		while(1)
		{
			T_EventMessage eventData;
			popEventData(eventData);
			
			//std::cout << "timeout: " << eventData.iTimeOut << std::endl;
			//std::cout << "trynum: " << eventData.iTryNum << std::endl;
			if (m_module)
			{
				std::string strResult = m_module->output(m_strIPCManageGUID,
					m_strIPCManageEventServer,
					(unsigned char*)eventData.strData.c_str(), 
					eventData.strData.length());

				if (strResult.length() == 0 && eventData.iTryNum > 0)
				{
					if (m_objEventManageSendFailedQueue)
					{
						eventData.iTryNum--;
						m_objEventManageSendFailedQueue->pushEventData(eventData);
					}
				}
			}

		}
	}

	bool CEventManageAttemptReport::pushEventData(
		T_EventMessage eventData)
	{
		return  m_tEventMessageQueue.push(eventData);
	}

	void CEventManageAttemptReport::popEventData(
		T_EventMessage &eventData)
	{
		m_tEventMessageQueue.pop(eventData);
	}

	int64_t CEventManageAttemptReport::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}
}