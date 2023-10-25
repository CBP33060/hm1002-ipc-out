#include "video_manage_channel.h"
#include "ai_manage_module.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "log_mx.h"
#include "cJSON.h"
#include "common.h"

namespace maix {
	CVideoManageChannel::CVideoManageChannel(CModule * module,
		std::string strName,
		CVideoSourceInputServer * objVideoSourceInputServer)
		: m_module(module)
		, m_strName(strName)
		, m_objVideoSourceInputServer(objVideoSourceInputServer)
		, m_AIObjectDetection(NULL)
		, m_AIPacketDetection(NULL)
		, m_AIRapidDetection(NULL)
	{
		
	}

	CVideoManageChannel::~CVideoManageChannel()
	{
	}

	mxbool CVideoManageChannel::init(std::shared_ptr<CEventManageRemoteEvent>
		eventManageRemoteEvent)
	{
		if (!m_module->getConfig("OBJECT_DETECTION", "CHN",
			m_strObjectDetectionCHN))
			return mxfalse;

		if (m_strName.compare(m_strObjectDetectionCHN) == 0)
		{
			if (!addObjectDetection(m_strName, eventManageRemoteEvent))
				return mxfalse;
		}

		// if (!m_module->getConfig("RAPID_DETECTION", "CHN", m_strRapidDetectionCHN))
		// 	return mxfalse;

		// if (m_strName.compare(m_strRapidDetectionCHN) == 0)
		// {
		// 	if (!addRapidDetection(m_strName))
		// 		return mxfalse;
		// }

		if (!getKey(m_strKey))
			return mxfalse;

		return mxtrue;
	}

	mxbool CVideoManageChannel::unInit()
	{
		if(m_AIObjectDetection)
			m_AIObjectDetection->unInit();
		return mxtrue;
	}

	mxbool CVideoManageChannel::syncDetectStayTime(int iStaytime)
	{
		if(m_AIObjectDetection)
			m_AIObjectDetection->setStayTime(iStaytime);
		return mxtrue;
	}

	void CVideoManageChannel::run()
	{
		while (1)
		{
			if (m_objVideoSourceInputServer)
			{
				std::shared_ptr<CMediaFramePacket> packet = NULL;
				m_objVideoSourceInputServer->popFrameData(packet);

				if (!packet)
				{
#ifdef	WIN32
					Sleep(500);
#else
					usleep(1000*500);
#endif
					continue;
				}

				if (packet->getPacketType() == E_P_VIDEO_YUV)
				{
					// if (m_strName.compare(m_strRapidDetectionCHN) == 0)
					// {
					// 	m_AIRapidDetection->pushFrameData(packet);
					// }

					if (m_strName.compare(m_strObjectDetectionCHN) == 0)
					{
						m_AIObjectDetection->pushFrameData(packet);
					}

				}

			}
			else
			{
#ifdef	WIN32
				Sleep(1000);
#else
				usleep(1000*1000);
#endif
			}
		}
	}

	mxbool CVideoManageChannel::addObjectDetection(std::string strName, 
		std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent)
	{
		std::shared_ptr<CAIObjectDetection> objectDetection(
			new CAIObjectDetection(m_module, strName, eventManageRemoteEvent));
		objectDetection->init();
		m_AIObjectDetection = objectDetection;
		m_threadObjectDetection = std::thread([objectDetection]() {
			objectDetection->run();
		});

		return mxtrue;
	}

	mxbool CVideoManageChannel::addPacketDetection(std::string strName,
		std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent,
		std::shared_ptr<CImgDetection> imgDetection)
	{
		std::shared_ptr<CAIPacketDetection> packetDetection(
			new CAIPacketDetection(m_module, strName, eventManageRemoteEvent, imgDetection));
		packetDetection->init();
		m_AIPacketDetection = packetDetection;
		m_threadPacketDetection = std::thread([packetDetection]() {
			packetDetection->run();
		});

		return mxtrue;
	}

	mxbool CVideoManageChannel::addRapidDetection(std::string strName)
	{
		std::shared_ptr<CAIRapidDetection> rapidDetection(
			new CAIRapidDetection(m_module, strName));

		rapidDetection->init();
		
		m_threadRapidDetection = std::thread([rapidDetection]() {
			rapidDetection->run();
		});

		m_AIRapidDetection = rapidDetection;
		return mxtrue;
	}

	void CVideoManageChannel::openDetect()
	{
		if (m_AIObjectDetection)
		{
			m_AIObjectDetection->openDetect();
		}
		if (m_AIPacketDetection)
		{
			m_AIPacketDetection->openDetect();
		}

	}

	void CVideoManageChannel::closeDetect()
	{
		if (m_AIObjectDetection)
		{
			m_AIObjectDetection->closeDetect();
		}
		if (m_AIPacketDetection)
		{
			m_AIPacketDetection->closeDetect();
		}

	}

}
