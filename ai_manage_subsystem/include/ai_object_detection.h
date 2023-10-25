#ifndef __AI_OBJECT_DETECTION_H__
#define __AI_OBJECT_DETECTION_H__
#include <opencv2/opencv.hpp>
#include "module.h"
#include "media_frame_packet.h"
#include "b_queue.h"
#include "event_mange_remote_event.h"
#include "image_detection.hpp"
#include "detector_result.h"

#define  YUV_FRAME_SIZE 			599040

#define  OBJECT_STAY_TIME_0			0	//经过
#define  OBJECT_STAY_TIME_1			3	//逗留3秒
#define  OBJECT_STAY_TIME_2			6	//逗留6秒

#define  STAYTIME_FRAME_NUM_1		5	//逗留3秒开始取帧数
#define  STAYTIME_FRAME_NUM_2		14	//逗留6秒开始取帧数

namespace maix {

	class MAIX_EXPORT CAIObjectDetection : public CAIDetect
	{
	public:
		CAIObjectDetection(CModule * module, std::string strName,
			std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent);
		~CAIObjectDetection();

		mxbool init();
		mxbool unInit();
		
		void run();

		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);

		void openDetect();
		void closeDetect();

		mxbool initImgDetection();
		mxbool setStayTime(int iStaytime);

		mxbool opencvIdentify(unsigned char* frameBuff, int frameSize, mxbool &bResult);
		void returnDetectResult(int iResult);

	private:
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		CModule * m_module;
		std::string m_strName;

		std::shared_ptr<CEventManageRemoteEvent> m_eventManageRemoteEvent;
		std::shared_ptr<CImgDetection> m_imgDetection;
		std::shared_ptr<CImgDetection> m_CImgDetection;

		mxbool m_bDetect;
		std::mutex m_mutex;

		mxbool m_bFrame1, m_bFrame2;
		cv::Mat m_matFrameGray1, m_matFrameGray2;
		int m_iConsecutivePositiveDiffCount;
		int m_iCountframe;
		mxbool m_bOpencvResult;
		int m_iRestFlag;
	};
}
#endif //__AI_OBJECT_DETECTION_H__
