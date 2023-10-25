#ifndef __EVENT_MANAGE_SEND_FAILED_QUEUE_H__
#define __EVENT_MANAGE_SEND_FAILED_QUEUE_H__
#include "module.h"
#include "b_queue.h"

namespace maix {
	typedef struct _t_EventMessage
	{
		int64_t iTimeOut;
		int		iTryNum;
		std::string strData;
	}T_EventMessage;

	class CEventManageAttemptReport;

	class CEventManageSendFailedQueue
	{
	public:
		CEventManageSendFailedQueue(
			CEventManageAttemptReport *objEventManageAttemptReport);
		~CEventManageSendFailedQueue();

		mxbool init();
		mxbool unInit();

		void run();

		bool pushEventData(T_EventMessage eventData);
		void popEventData(T_EventMessage &eventData);

		int64_t getCurrentTime();

	private:
		CBQueue<T_EventMessage> m_tEventMessageQueue;
		CEventManageAttemptReport *m_objEventManageAttemptReport;
	};
}
#endif //__EVENT_MANAGE_SEND_FAILED_QUEUE_H__
