#include "audio_source_input.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "log_mx.h"

namespace maix {
	CAudioSourceInput::CAudioSourceInput(int iChnSN, std::string strName, 
		E_P_TYPE ePacketType,
		CModule * module, std::shared_ptr<CMediaInterface> objInterface)
		: m_module(module)
	{
		m_iChnSN = iChnSN;
		m_ePacketType = ePacketType;
		m_strName = strName;
		m_interface = objInterface;
		m_iLostFrameNum = 0;
	}

	CAudioSourceInput::~CAudioSourceInput()
	{
	}

	mxbool CAudioSourceInput::init(
		std::shared_ptr<CAudioSourceChannel> channel)
	{
		m_channel = channel;
		return m_interface->initChannel(m_iChnSN);
	}

	mxbool CAudioSourceInput::unInit()
	{
		return mxbool();
	}

	void CAudioSourceInput::run()
	{
		while (1)
		{
			int size = 0;
			int frameType = 0;
			int64_t frameTimeStamp = 0;
			int frameSeq = 0;
			unsigned char * framedata = m_interface->readFrame( m_iChnSN, &size, &frameType, &frameTimeStamp, &frameSeq);
			// logPrint(MX_LOG_DEBUG, "audio channel %s frameTimeStamp: %lld", m_strName.c_str(), frameTimeStamp);

			if (size > 0)
			{
				std::shared_ptr<CMediaFramePacket>  packet(
					new CMediaFramePacket());
				packet->setPacketType(m_ePacketType);

				if (!packet->setFrameData(framedata, size, frameTimeStamp, frameSeq))
					return;
				
				if (!m_channel->pushFrameData(packet))
				{
					std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
					m_channel->popFrameData(lostPacket);
					m_iLostFrameNum++;
					//logPrint(MX_LOG_ERROR, "channel %s lost frame num: %d",
					//	m_strName.c_str(), m_iLostFrameNum);

					if (!m_channel->pushFrameData(packet))
					{
						logPrint(MX_LOG_ERROR, "channel %s insert frame error",
							m_strName.c_str());
					}
				}
			}
		}
	}

	int64_t CAudioSourceInput::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

}
