#include "speaker_source_channel.h"
#include "media_frame_packet.h"
#include "log_mx.h"

namespace maix {
	CSpeakerSourceChannel::CSpeakerSourceChannel(int iChnSN, 
		std::string strName, 
		E_P_TYPE ePacketType,
		CSpeakerSourceInputServer * objSourceInput, 
		std::shared_ptr<CMediaInterface> objInterface)
		: m_objSourceInput(objSourceInput)
	{
		m_iChnSN = iChnSN;
		m_strName = strName;
		m_ePacketType = ePacketType;
		m_interface = objInterface;

		if (m_objSourceInput)
			m_objSourceInput->addInterface(objInterface);
	}

	CSpeakerSourceChannel::~CSpeakerSourceChannel()
	{
	}

	mxbool CSpeakerSourceChannel::init()
	{
		if (!m_objSourceInput)
			return mxfalse;

		return m_interface->initChannel(m_iChnSN);
	}

	mxbool CSpeakerSourceChannel::unInit()
	{
		return m_interface->unInitChannel(m_iChnSN);
	}

	void CSpeakerSourceChannel::run()
	{
		while (1)
		{
			if (m_objSourceInput)
			{
				std::shared_ptr<CMediaFramePacket> packet = NULL;
				m_objSourceInput->popFrameData(packet);

				if (!packet)
				{
#ifdef	WIN32
 					Sleep(30);
#else
 					usleep(30*1000);
#endif  
                    continue;
				}

				T_MediaFramePacketHeader tMediaFramePacketHeader;
				int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
				if (packet->getFrameDataLen() > iPacketHeaderLen)
				{
					memcpy(&tMediaFramePacketHeader, packet->getFrameData(),
						iPacketHeaderLen);
				}
				else
				{
					continue;
				}

				
				if (packet->getPacketType() == m_ePacketType)
				{
					if (!m_interface->writeFrame(m_iChnSN,
						packet->getFrameData() + iPacketHeaderLen,
						tMediaFramePacketHeader.nPacketSize))
					{
						logPrint(MX_LOG_DEBUG, "interface <%s> write failed",
							m_strName.c_str());
					}
				}
				else
				{
					logPrint(MX_LOG_DEBUG, "interface <%s> TYPE is error",
						m_strName.c_str());
				}
				
			}
			
		}
	}
}
