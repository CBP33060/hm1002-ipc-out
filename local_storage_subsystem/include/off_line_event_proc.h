#ifndef __OFF_LINE_EVENT_PROC_H__
#define __OFF_LINE_EVENT_PROC_H__
#include "module.h"
#include "b_queue.h"
#include "media_frame_packet.h"
#include "timer_mx.h"
#include <mp4v2/mp4v2.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

namespace maix {
	#define VIDEO_FPS 15
	typedef enum __H264_NALU_TYPE_E{
		H264_NALU_TYPE_NONE = -1,
		H264_NALU_TYPE_SLICE = 0X01,
		H264_NALU_TYPE_IDR = 0X05,
		H264_NALU_TYPE_SPS = 0x07,
		H264_NALU_TYPE_PPS = 0x08,
		H264_NALU_TYPE_MASK = 0x1f
	} H264_NALU_TYPE_E;

	typedef enum __H265_NALU_TYPE_E{
		H265_NALU_TYPE_NONE = -1,
		H265_NALU_TYPE_IDR = 19,
		H265_NALU_TYPE_SPS = 33,
		H265_NALU_TYPE_PPS = 34,
		H265_NALU_TYPE_MASK = 0x7e
	} H265_NALU_TYPE_E;

	const int timeScale = 90000;
	class COffLineEventProc
	{
	public:
		COffLineEventProc(CModule * module);
		~COffLineEventProc();

		mxbool init(int iRecordDuration);
		mxbool unInit();

		bool pushFrameData(std::shared_ptr<CMediaFramePacket> 
			&packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> 
			&packet);

		void run();

		mxbool startRecord();
		mxbool stopRecord();
		int writeMP4VideoFrame(unsigned char* data, int len);
		int writeMP4VideoSPS(unsigned char* data, int len);
		int writeMP4VideoPPS(unsigned char* data, int len);
		int writeMP4VideoH264(unsigned char* data, int len);
		int writeMP4AudioAACFrame(unsigned char* data, int len);

		mxbool startCreateMp4(const std::string& mp4FileName);
		mxbool stopCreateMp4();

		mxbool writeMP4VideoFrameV2(int64_t timeStamp, unsigned char* pdata, int iLen);
		mxbool writeMP4AudioFrameV2(int64_t timeStamp, unsigned char* pdata, int iLen);
		mxbool isH265KeyFrame(unsigned char* pdata, int iLen);

	private:
		CModule *m_module;
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		int m_iRecordDuration;
		CTimer m_timerRecord;
		mxbool m_bRecordStart;

		MP4FileHandle m_pMP4FileHandle;
		MP4TrackId m_MP4VideoID;
		MP4TrackId m_MP4AudioID;
		mxbool m_bAddVideoTrack;
		mxbool m_bAddAudioTrack;

		AVFormatContext* m_mp4FormatCtx;
		AVCodecContext * m_inputVideoCtx;
		AVCodecContext * m_inputAudioCtx;

		AVStream* m_videoInputStream;
		AVStream* m_audioInputStream;

		mxbool m_bWaitIDR;
		int m_iVideoIndex;
		int m_iAudioIndex;
		int m_frameVideoIndex;
		int m_frameAudioIndex;
		int m_fps;
	};
}
#endif //__OFF_LINE_EVENT_PROC_H__
