#ifndef __AI_RAPID_DETECTION_H__
#define __AI_RAPID_DETECTION_H__
#include "module.h"
#include "media_frame_packet.h"
#include "b_queue.h"
#include <opencv2/opencv.hpp>

namespace maix {
	class MAIX_EXPORT CAIRapidDetection
	{
	public:
		CAIRapidDetection(CModule * module, std::string strName);
		~CAIRapidDetection();

		mxbool init();
		mxbool unInit();
		
		void run();
		mxbool opencvIdentify(unsigned char* frameBuff, int frameSize, mxbool &bResult);
		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);

	private:
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		CModule * m_module;
		std::string m_strName;

		mxbool m_bFrame1, m_bFrame2;
		cv::Mat m_matFrameGray1, m_matFrameGray2;
		int m_iConsecutivePositiveDiffCount;
		int m_iCountframe;
		int m_iRestFlag;
	};
}
#endif //__AI_RAPID_DETECTION_H__
