#ifndef __VIDEO_SOURCE_CHANNEL_H__
#define __VIDEO_SOURCE_CHANNEL_H__
#include "module.h"
#include "media_interface.h"
#include "b_queue.h"
#include "media_frame_packet.h"
#include <list>

namespace maix {
	class MAIX_EXPORT CVideoSourceChannel
	{
	public:
		CVideoSourceChannel(std::string strName, E_P_TYPE ePacketType, CModule* module,void *objVideoSourceModule);
		~CVideoSourceChannel();

		mxbool init();
		mxbool unInit();
		void run();

		mxbool open(std::string strGUID, std::string strServerName);
		mxbool close(std::string strGUID, std::string strServerName);
		mxbool config(std::string strConfig);
		mxbool reset();

		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		
		mxbool getJpegFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		int64_t getCurrentTime();
        void handleEventJpeg();
        int getMapClientNum();

	private: 
		mxbool m_jpeg;
		std::string m_strName;
		E_P_TYPE m_ePacketType;
		CModule* m_module;
		std::mutex m_mutexClient;
		std::map<std::string, std::list<std::string>> m_mapClient;
		void *m_objVideoSourceModule;
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		int m_iClientNumCount;
	};
}
#endif //__VIDEO_SOURCE_CHANNEL_H__
