#ifndef __AI_PACKET_DETECTION_H__
#define __AI_PACKET_DETECTION_H__
#include "module.h"
#include "media_frame_packet.h"
#include "b_queue.h"
#include "event_mange_remote_event.h"
#include "image_detection.hpp"
namespace maix {
	class MAIX_EXPORT CAIPacketDetection
	{
	public:
		CAIPacketDetection(CModule * module, std::string strName,
			std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent,
			std::shared_ptr<CImgDetection> imgDetection);
		~CAIPacketDetection();

		mxbool init();
		mxbool unInit();

		void run();

		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);

		void openDetect();
		void closeDetect();

	private:
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		CModule * m_module;
		std::string m_strName;

		std::shared_ptr<CEventManageRemoteEvent> m_eventManageRemoteEvent;
		std::shared_ptr<CImgDetection> m_imgDetection;

		mxbool m_bDetect;
		std::mutex m_mutex;
	};
}
#endif //__AI_PACKET_DETECTION_H__
