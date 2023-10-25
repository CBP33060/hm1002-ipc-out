#include "event_manage_send_failed_queue.h"
#include "event_manage_attempt_report.h"
#ifdef _WIN32
#include <sys/timeb.h>
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace maix {
	CEventManageSendFailedQueue::CEventManageSendFailedQueue(
		CEventManageAttemptReport *objEventManageAttemptReport)
		: m_objEventManageAttemptReport(objEventManageAttemptReport)
	{
	}

	CEventManageSendFailedQueue::~CEventManageSendFailedQueue()
	{
	}

	mxbool CEventManageSendFailedQueue::init()
	{
		if (!m_tEventMessageQueue.init(10, 0))
			return mxfalse;

		return mxtrue;
	}

	mxbool CEventManageSendFailedQueue::unInit()
	{
		return mxbool();
	}

	void CEventManageSendFailedQueue::run()
	{
		while (1)
		{
			T_EventMessage eventData;
			popEventData(eventData);

			std::cout << "try timeout: " << eventData.iTimeOut << std::endl;
			std::cout << "try trynum: " << eventData.iTryNum << std::endl;
			if (m_objEventManageAttemptReport)
			{
				int64_t now = getCurrentTime();
				if (eventData.iTimeOut > now)
				{
					m_objEventManageAttemptReport->pushEventData(eventData);
					continue;
				}
				
			}
#ifdef	WIN32
			Sleep(EVENT_TRY_INTERVAL * 1000);
#else
			usleep(EVENT_TRY_INTERVAL *1000 * 1000);
#endif
		}
	}

	bool CEventManageSendFailedQueue::pushEventData(T_EventMessage eventData)
	{
		return  m_tEventMessageQueue.push(eventData);
	}

	void CEventManageSendFailedQueue::popEventData(T_EventMessage &eventData)
	{
		m_tEventMessageQueue.pop(eventData);
	}

	int64_t CEventManageSendFailedQueue::getCurrentTime()
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
