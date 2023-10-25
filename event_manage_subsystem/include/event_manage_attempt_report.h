#ifndef __EVENT_MANAGE_ATTEMPT_REPORT_H__
#define __EVENT_MANAGE_ATTEMPT_REPORT_H__
#include <memory>
#include "module.h"
#include "b_queue.h"
#include "event_manage_send_failed_queue.h"

#define EVENT_TRY_NUM  3
#define EVENT_TRY_INTERVAL 5

namespace maix {
	class MAIX_EXPORT CEventManageAttemptReport
	{
	public:
		CEventManageAttemptReport(CModule * module);
		~CEventManageAttemptReport();

		mxbool init();
		mxbool unInit();

		void run();

		bool pushEventData(T_EventMessage eventData);
		void popEventData(T_EventMessage &eventData);

		int64_t getCurrentTime();

	private:
		CBQueue<T_EventMessage> m_tEventMessageQueue;
		CModule * m_module;
		std::shared_ptr<CEventManageSendFailedQueue> m_objEventManageSendFailedQueue;

		std::string m_strIPCManageGUID;
		std::string m_strIPCManageEventServer;

		std::thread m_threadEventManageFailedQueue;
	};
}
#endif //__EVENT_MANAGE_ATTEMPT_REPORT_H__
