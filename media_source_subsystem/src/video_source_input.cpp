#include "video_source_input.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include "log_mx.h"
#include "cJSON.h"
 
namespace maix {
	CVideoSourceInput::CVideoSourceInput(int iChnSN, std::string strName, 
		E_P_TYPE ePacketType,
		CModule * module, std::shared_ptr<CMediaInterface> objInterface)
		: m_module(module)
	{
		m_iChnSN = iChnSN;
		m_strName = strName;
		m_ePacketType = ePacketType;
		m_interface = objInterface;
		m_iLostFrameNum = 0;
	}

	CVideoSourceInput::~CVideoSourceInput()
	{
	}

	mxbool CVideoSourceInput::init(
		std::shared_ptr<CVideoSourceChannel> channel)
	{	
		m_channel = channel;
		return m_interface->initChannel(m_iChnSN);
	}

	mxbool CVideoSourceInput::unInit()
	{
		return mxbool();
	}

    mxbool CVideoSourceInput::getIDRframe()
    {
        logPrint(MX_LOG_ERROR, "channel %s get IDR Frame",m_strName.c_str());
        if(!m_interface->getIDRFrame(m_iChnSN))
        {
            logPrint(MX_LOG_ERROR, "channel %s get IDR Frame fail!!!",m_strName.c_str());
            return mxtrue;
        }
        return mxfalse;
    }

    mxbool CVideoSourceInput::getJpegFrameData()
    {
        logPrint(MX_LOG_ERROR, "channel %s getJpegFrameData",m_strName.c_str());
		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonParam, "configName", "GetJpegCover");

		cJSON *jsonValue = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonValue, "value", "0");

		cJSON_AddItemToObject(pJsonParam, "configValue", jsonValue);

        char *out = cJSON_PrintUnformatted(pJsonParam);
        std::string strParam = std::string(out);

        if(!m_interface->config(strParam))
        {
            logPrint(MX_LOG_ERROR, "channel %s getJpegFrameData fail!!!",m_strName.c_str());
            return mxfalse;
        }
        if(m_channel)
        {   
            m_channel->handleEventJpeg();
        }
        return mxtrue;
    }

	void CVideoSourceInput::run()
	{
		if(m_iChnSN >=0 && m_iChnSN <= 4)
		{
			if(!m_interface->startRcvFrame(m_iChnSN))
			{
				logPrint(MX_LOG_ERROR, "channel %s start frame fail!!!",m_strName.c_str());
				return;
			}
		}
		while (1)
		{
			int size = 0;
			int index = 0;
			int frameType = 0;
			int64_t frameTimeStamp = 0;
			int frameSeq = 0;
			unsigned char *framedata = m_interface->readFrame(m_iChnSN, &size, &frameType, &frameTimeStamp, &frameSeq);
			
			E_F_TYPE eFrameType = E_F_NULL;
			if(frameType == 0)
				eFrameType = E_F_P;
			else if(frameType == 1)
				eFrameType = E_F_I;
			else if(frameType == 2)
				eFrameType = E_F_B;

			if (size > 0)
			{
				while ((unsigned int)size > MEDIA_FRAME_PACKET_LEN - sizeof(T_MediaFramePacketHeader))
				{
					std::shared_ptr<CMediaFramePacket>  packet(
						new CMediaFramePacket());
					packet->setPacketType(m_ePacketType);
					packet->setFrameType(eFrameType);
					packet->setPacketMark(0);

					if (!packet->setFrameData(framedata + index,
								MEDIA_FRAME_PACKET_LEN - sizeof(T_MediaFramePacketHeader), 
								frameTimeStamp, frameSeq))
						return;

					if (!m_channel->pushFrameData(packet))
					{
						std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
						m_channel->popFrameData(lostPacket);
						m_iLostFrameNum++;
                        // if(m_strName.compare("video_channel_1_server") == 0)
                        // {
                        //     logPrint(MX_LOG_ERROR, "channel %s lost frame num: %d",
						// 		m_strName.c_str(), m_iLostFrameNum);
                        // }

						if (!m_channel->pushFrameData(packet))
						{
							logPrint(MX_LOG_ERROR, "channel %s insert frame error",
								m_strName.c_str());
						}
						
					}

					size = size - MEDIA_FRAME_PACKET_LEN + sizeof(T_MediaFramePacketHeader);
					index = index + MEDIA_FRAME_PACKET_LEN - sizeof(T_MediaFramePacketHeader);
					usleep(10);
				}

				if (size > 0)
				{
					std::shared_ptr<CMediaFramePacket>  packet(
						new CMediaFramePacket());
					packet->setPacketType(m_ePacketType);
					packet->setFrameType(eFrameType);
					packet->setPacketMark(1);
					if (!packet->setFrameData(framedata + index, size, frameTimeStamp, frameSeq))
						return;

					if (!m_channel->pushFrameData(packet))
					{
						std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
						m_channel->popFrameData(lostPacket);
						m_iLostFrameNum++;

                        // if(m_strName.compare("video_channel_1_server") == 0)
                        // {
                        //     logPrint(MX_LOG_ERROR, "channel %s mark 1 lost frame num: %d",
						// 		m_strName.c_str(), m_iLostFrameNum);
                        // }

						// if(m_iLostFrameNum % 15 == 0)
						// {
						// 	if(!m_interface->getIDRFrame(m_iChnSN))
						// 	{
						// 		logPrint(MX_LOG_ERROR, "channel %s get IDR Frame fail!!!",m_strName.c_str());
						// 		return;
						// 	}
						// }

						if (!m_channel->pushFrameData(packet))
						{
							// logPrint(MX_LOG_ERROR, "channel %s insert frame error",
							// 	m_strName.c_str());
						}
					}
				}
			}
		}
	}
	int64_t CVideoSourceInput::getCurrentTime()
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
