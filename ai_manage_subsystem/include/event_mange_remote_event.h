#ifndef __EVENT_MANAGE_REMOTE_EVENT_H__
#define __EVENT_MANAGE_REMOTE_EVENT_H__
#include "module.h"
#include "b_queue.h"

namespace maix {

	class MAIX_EXPORT CEventManageRemoteEvent
	{
	public:
		CEventManageRemoteEvent(CModule * module);
		~CEventManageRemoteEvent();

		mxbool init(std::string strGUID, std::string strServer);
		mxbool unInit();

		void run();

		bool pushFrameData(std::string strEvent);
		void popFrameData(std::string &strEvent);

		void openAIEventPush();
		void closeAIEventPush();

		void sendAlarmEvent(std::string strEventValue, std::string strEventType);

	private:
		CBQueue<std::string> m_objEventQueue;
		CModule * m_module;
		std::string m_strGUID;
		std::string m_strServer;
		std::string m_strDID;

		mxbool m_bDetect;
		std::mutex m_mutex;
	};
}
#endif //__EVENT_MANAGE_REMOTE_EVENT_H__
