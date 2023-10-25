#include "ai_packet_detection.h"
#include "ai_manage_module.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "log_mx.h"
namespace maix {
	CAIPacketDetection::CAIPacketDetection(CModule * module, 
		std::string strName,
		std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent,
		std::shared_ptr<CImgDetection> imgDetection)
		: m_module(module)
		, m_strName(strName)
		, m_eventManageRemoteEvent(eventManageRemoteEvent)
		, m_imgDetection(imgDetection)
	{
		m_bDetect = mxtrue;
	}

	CAIPacketDetection::~CAIPacketDetection()
	{
	}

	mxbool CAIPacketDetection::init()
	{
		if (!m_objPacketQueue.init(3, 1000))
			return mxfalse;

		// if (!m_imgDetection->loadBinModel(AI_PD_MODEL_PATH))
		// 	return mxfalse;

		return mxtrue;
	}

	mxbool CAIPacketDetection::unInit()
	{
		return mxbool();
	}

	void CAIPacketDetection::run()
	{
		int iResult = 0;
		int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
		CAIManageModule *module = dynamic_cast<CAIManageModule *>(m_module);

		while (1)
		{
			std::shared_ptr<CMediaFramePacket> packet = NULL;
			popFrameData(packet);

			std::unique_lock<std::mutex> lock(m_mutex);	
			if (!packet  || !m_bDetect)
			{
#ifdef	WIN32
				Sleep(500);
#else
				usleep(1000*500);
#endif
				continue;
			}
			lock.unlock();

			if (packet->getPacketType() == E_P_VIDEO_YUV)
			{
				logPrint(MX_LOG_DEBUG, "packet detection data len: %d, Staytime:%d, AreaDetecMask.size=%d",
					packet->getFrameDataLen() - iPacketHeaderLen,
					module->getStaytime(), module->getAreaDetecMask().size());
					
				iResult = m_imgDetection->procFrameData(packet->getFrameData()+iPacketHeaderLen, 
																packet->getFrameDataLen()-iPacketHeaderLen, 1);

				if(iResult > 0)
				{
					logPrint(MX_LOG_DEBUG, "packet detection data result: %d", iResult);
					int maskdetect = module->getDetectMask();
					for (int i = 0; i < E_EVENT_TYPE_MAX; i++)
					{
						if ((iResult & (1 << i)) &maskdetect)
						{
							//event
							m_eventManageRemoteEvent->pushFrameData(std::to_string(i));	
						}
					}
				}
			}

		}
	}

	bool CAIPacketDetection::pushFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CAIPacketDetection::popFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		m_objPacketQueue.pop(packet);
	}

	void CAIPacketDetection::openDetect()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bDetect = mxtrue;

		logPrint(MX_LOG_DEBUG, "open ai pack detection");
	}

	void CAIPacketDetection::closeDetect()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bDetect = mxfalse;

		logPrint(MX_LOG_DEBUG, "close ai pack detection");
	}
}
