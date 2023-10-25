#ifndef __VIDEO_MANAGE_CHANNEL_H__
#define __VIDEO_MANAGE_CHANNEL_H__
#include "module.h"
#include "video_source_input_server.h"
#include <list>
#include "ai_object_detection.h"
#include "ai_packet_detection.h"
#include "ai_rapid_detection.h"
#include "event_mange_remote_event.h"
#include "image_detection.hpp"

namespace maix {

	class MAIX_EXPORT CVideoManageChannel
	{
	public:
		CVideoManageChannel(CModule * module, std::string strName,
			CVideoSourceInputServer* objVideoSourceInputServer);
		~CVideoManageChannel();

		mxbool init(std::shared_ptr<CEventManageRemoteEvent>
			eventManageRemoteEvent);

		mxbool unInit();
		mxbool syncDetectStayTime(int iStaytime);

		void run();

		mxbool addObjectDetection(std::string strName,
			std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent);
		mxbool addPacketDetection(std::string strName,
			std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent,
			std::shared_ptr<CImgDetection> imgDetection);
		mxbool addRapidDetection(std::string strName);

		void openDetect();
		void closeDetect();
	private:
		CModule * m_module;
		std::string m_strName;
		std::string m_strKey;
		CVideoSourceInputServer* m_objVideoSourceInputServer;

		std::string m_strObjectDetectionCHN;
		std::thread m_threadObjectDetection;
		std::shared_ptr<CAIObjectDetection> m_AIObjectDetection;

		std::string m_strPacketDetectionCHN;
		std::thread m_threadPacketDetection;
		std::shared_ptr<CAIPacketDetection> m_AIPacketDetection;

		std::string m_strRapidDetectionCHN;
		std::thread m_threadRapidDetection;
		std::shared_ptr<CAIRapidDetection> m_AIRapidDetection;
	};
}
#endif //__VIDEO_MANAGE_CHANNEL_H__
