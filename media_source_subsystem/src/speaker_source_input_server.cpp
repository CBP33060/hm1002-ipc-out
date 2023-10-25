#include "speaker_source_input_server.h"
#include <iostream>
#include "media_frame_packet.h"
#include "cJSON.h"
#include "log_mx.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#define AUDIO_INTERVAL_TIME         200         //单位：ms

namespace maix {
	CSpeakerSourceInputServer::CSpeakerSourceInputServer(CModule * module)
		: m_module(module)
		, m_iLostFrameNum(0)
	{
        m_objAudioRes = E_SOURCE_FILE;
        m_objAudioLevel = E_LEVEL_LOWEST;
        m_llAudioLastTime = 0;
        m_bSpeakerInit = mxtrue;
	}

	CSpeakerSourceInputServer::~CSpeakerSourceInputServer()
	{
	}

	mxbool CSpeakerSourceInputServer::init(std::string strChannelName,
		int iType,
		std::string strIP,
		int iPort,
		std::string strUnix,
		int iLen)
	{
		if (!m_objPacketQueue.init(45, 30))
			return mxfalse;


		if (!m_module->getConfig("MODULE", "GUID", m_strModuleGUID))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "NAME", m_strModuleName))
			return mxfalse;

		m_strChannelName = strChannelName;
		m_iType = iType;
		m_strIP = strIP;
		m_iPort = iPort;
		m_strUnix = strUnix;
		m_iLen = iLen;

        m_threadSpeakerControl = std::thread([this]() {

			this->runAsSpeakerControl();
		});
        m_threadSpeakerControl.detach();

		return mxtrue;
	}

	mxbool CSpeakerSourceInputServer::unInit()
	{
		return mxbool();
	}

	mxbool CSpeakerSourceInputServer::addInterface(
		std::shared_ptr<CMediaInterface> objInterface)
	{
		m_objInterface = objInterface;
		return mxtrue;
	}

	void CSpeakerSourceInputServer::frameProc(RCF::ByteBuffer byteBuffer)
	{
		int iDataSize = byteBuffer.getLength();
		if (iDataSize > 0)
		{
			std::shared_ptr<CMediaFramePacket>  packet(
				new CMediaFramePacket());
			int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
			if (iDataSize > iPacketHeaderLen)
			{
				T_MediaFramePacketHeader tMediaFramePacketHeader;
				memcpy(&tMediaFramePacketHeader, byteBuffer.getPtr(), iPacketHeaderLen);
				char *pcData = byteBuffer.getPtr() + iPacketHeaderLen;

				packet->setPacketType(tMediaFramePacketHeader.ePacketType);
                packet->setReserve_1(tMediaFramePacketHeader.iReserve_1);
                packet->setReserve_2(tMediaFramePacketHeader.iReserve_2);

				if (!packet->setFrameData((unsigned char*)pcData,
					tMediaFramePacketHeader.nPacketSize,
					tMediaFramePacketHeader.lTimeStamp,
					tMediaFramePacketHeader.iFrameSeq))
					return;

				logPrint(MX_LOG_DEBUG, "<%s> send frame len: %d ireserver_1 %d ireserver_2 %d",
					m_strChannelName.c_str(), packet->getFrameDataLen(),packet->getReserve_1(),packet->getReserve_2());

                int64_t currentTime = getCurrentTime();
                // logPrint(MX_LOG_ERROR,"current time %llu \n",currentTime);                  
                //p2p与file播报的优先级规则
                //两帧之间的差距大于AUDIO_INTERVAL_TIME，直接进行播放
                //两帧之间的差距小于AUDIO_INTERVAL_TIME，判断当前帧是否和上一帧的来源相同，如果相同，则直接播放
                //两帧来源不同：判断当前帧如果时P2P帧，上一帧的level如果大于P2P帧的level，则当前帧不播放直接跳过。
                //如果当前帧不是P2P帧，则说明当前帧时file帧，当前帧如果大于上一帧的level，则播放，否则跳过。
                if((currentTime - m_llAudioLastTime) < AUDIO_INTERVAL_TIME)
                {
                    if(m_objAudioRes != packet->getReserve_1())
                    {
                        if(E_SOURCE_P2P == packet->getReserve_1())
                        {
                            if(m_objAudioLevel > packet->getReserve_2())
                            {
                                // logPrint(MX_LOG_ERROR,"P2P setReserve_1 %d  setReserve_2 %d\n",packet->getReserve_1(),tMediaFramePacketHeader.iReserve_2);
                                return;
                            }
                        }
                        else
                        {
                            if(m_objAudioLevel >=  packet->getReserve_2())
                            {
                                // logPrint(MX_LOG_ERROR,"file  setReserve_1 %d  setReserve_2 %d\n",packet->getReserve_1(),tMediaFramePacketHeader.iReserve_2);
                                return;
                            }
                        }
                    }
                }
                m_objAudioRes = (E_VOICE_SOURCE) packet->getReserve_1();
                m_objAudioLevel = (E_VOICE_PLAY_LEVEL) packet->getReserve_2();
                m_llAudioLastTime = currentTime;

				if (!pushFrameData(packet))
				{
					std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
					popFrameData(lostPacket);
					m_iLostFrameNum++;
					logPrint(MX_LOG_DEBUG, "<%s> lost frame num: %d",
						m_strChannelName.c_str(), m_iLostFrameNum);

					if (!pushFrameData(packet))
					{
						logPrint(MX_LOG_DEBUG, "channel %s insert frame error",
							m_strChannelName.c_str());
					}
				}
			}
		}

	}

	bool CSpeakerSourceInputServer::pushFrameData(
		std::shared_ptr<CMediaFramePacket> &packet)
	{
        std::unique_lock<std::mutex> lock(m_mutexSpeakerControl);
        m_conditionSpeakerControl.notify_one();
		return  m_objPacketQueue.push(packet);
	}

	void CSpeakerSourceInputServer::popFrameData(
		std::shared_ptr<CMediaFramePacket> &packet)
	{
		m_objPacketQueue.pop(packet);
	}

	std::string CSpeakerSourceInputServer::open(std::string strParam)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "openSpeaker");

		cJSON_AddStringToObject(jsonParam, "moduleGUID", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "moduleName", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "serverName", m_strChannelName.c_str());
		cJSON_AddStringToObject(jsonParam, "channelID", m_strChannelName.c_str());

		cJSON_AddNumberToObject(jsonParam, "type", m_iType);
		cJSON_AddStringToObject(jsonParam, "ip", m_strIP.c_str());
		cJSON_AddNumberToObject(jsonParam, "port", m_iPort);
		cJSON_AddStringToObject(jsonParam, "unix", m_strUnix.c_str());
		cJSON_AddNumberToObject(jsonParam, "len", m_iLen);


		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strOpen = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		return procResult("200", strOpen, "");
	}

	std::string CSpeakerSourceInputServer::close(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerSourceInputServer::config(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerSourceInputServer::reset(std::string strParam)
	{
		return std::string();
	}

	std::string CSpeakerSourceInputServer::procResult(
		std::string code, std::string strMsg, std::string strErr)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonMsg = cJSON_Parse(strMsg.c_str());
		cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
		cJSON_AddItemToObject(jsonRoot, "msg", jsonMsg);
		cJSON_AddStringToObject(jsonRoot, "errMsg", strErr.c_str());
		char *pcResult = cJSON_Print(jsonRoot);
		strResult = std::string(pcResult);
		cJSON_Delete(jsonRoot);
		if (pcResult)
			free(pcResult);

		return strResult;
	}

    void CSpeakerSourceInputServer::runAsSpeakerControl()
	{
        unsigned long long m_llSpeakerSystemTime;       //由于系统修改时间从1970到2023，导致wait_until跳出wait状态后，返回值时超时，
                                            //导致业务不正常，所以增加这个全局变量存储时间，超时后判断时间跨度过大，认为超时无效。

        while (1)
        {
            {
                bool bOutTime = false;
                
                std::unique_lock<std::mutex> lock(m_mutexSpeakerControl);
                if(m_bSpeakerInit)
                {
                    m_llSpeakerSystemTime = getCurrentTime();
                    
                    std::cv_status ret = m_conditionSpeakerControl.wait_until(lock,std::chrono::steady_clock::now() + std::chrono::milliseconds(2 * 1000));
                    if(ret == std::cv_status::timeout)
                    {
                        unsigned long long waitTime = getCurrentTime();
                        if((unsigned long long)(waitTime - m_llSpeakerSystemTime) <= (3 * 1000) )
                        {
                            bOutTime = true;
                        }
                        else
                        {
                            logPrint(MX_LOG_INFOR,"CSpeakerSourceInputServer time out err");
                        }
                    }
                    else
                    {
                        bOutTime = false;
                    }
                }else
                {
                    m_conditionSpeakerControl.wait(lock);
                    bOutTime = false;
                }
                
                if(bOutTime && m_objPacketQueue.isEmpty())
                {
                    if(!m_bSpeakerInit)
                    {
                        continue;
                    }
                    if(m_objInterface && m_objInterface->unInit())
                    {
                        m_bSpeakerInit = mxfalse;
                    }
                }
                else
                {
                    if(m_bSpeakerInit)
                    {
                        continue;
                    }
                    if(m_objInterface && m_objInterface->init())
                    {
                        m_bSpeakerInit = mxtrue;
                    }
                }

            }
        }
	}

    unsigned long long CSpeakerSourceInputServer::getCurrentTime()
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
