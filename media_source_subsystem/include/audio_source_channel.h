#ifndef __AUDIO_SOURCE_CHANNEL_H__
#define __AUDIO_SOURCE_CHANNEL_H__
#include "module.h"
#include "media_interface.h"
#include "b_queue.h"
#include "media_frame_packet.h"
#include <list>

namespace maix {
	class MAIX_EXPORT CAudioSourceChannel
	{
	public:
		CAudioSourceChannel(std::string strName, CModule* module);
		~CAudioSourceChannel();

		mxbool init();
		mxbool unInit();
		void run();

		mxbool open(std::string strGUID, std::string strServerName);
		mxbool close(std::string strGUID, std::string strServerName);
		mxbool config(std::string strConfig);
		mxbool reset();

		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);

	private:
		std::string m_strName;
		CModule* m_module;
		std::mutex m_mutexClient;
		std::map<std::string, std::list<std::string>> m_mapClient;

		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
	};
}
#endif //__AUDIO_SOURCE_CHANNEL_H__
