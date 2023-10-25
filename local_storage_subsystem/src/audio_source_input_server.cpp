#include "audio_source_input_server.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <iostream>
#include "media_frame_packet.h"
#include "audio_manage_channel.h"
#include "log_mx.h"
#include "crypt_api_mx.h"
#include "cJSON.h"
#include "common.h"


namespace maix {
	CAudioSourceInputServer::CAudioSourceInputServer(CModule * module)
		: m_module(module)
		, m_iPacketReceiveTime(0)
		, m_iLostFrameNum(0)
	{
	
	}

	CAudioSourceInputServer::~CAudioSourceInputServer()
	{
	}

	mxbool CAudioSourceInputServer::init(std::string strName,
		int iType,
		std::string strIP,
		int iPort,
		std::string strUnix,
		int iLen)
	{
		if (!m_objPacketQueue.init(10, 1000))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "GUID", m_strModuleGUID))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "NAME", m_strModuleName))
			return mxfalse;

		if (!initAudioManageRemoteEventServer())
			return mxfalse;

		m_strChannelName = strName;
		m_iType = iType;
		m_strIP = strIP;
		if (m_strIP.compare("0.0.0.0") == 0)
			m_strIP = std::string("127.0.0.1");
		m_iPort = iPort;
		m_strUnix = strUnix;
		m_iLen = iLen;

		return mxtrue;
	}

	mxbool CAudioSourceInputServer::unInit()
	{
		return mxbool();
	}

	void CAudioSourceInputServer::frameProc(RCF::ByteBuffer byteBuffer)
	{
		updatePacketTime();

		int iDataSize = byteBuffer.getLength();
		if (iDataSize > 0)
		{
			std::shared_ptr<CMediaFramePacket>  packet(
				new CMediaFramePacket());
			int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
			if (iDataSize > iPacketHeaderLen)
			{
				T_MediaFramePacketHeader tMediaFramePacketHeader;
				memcpy(&tMediaFramePacketHeader, byteBuffer.getPtr(), 
					iPacketHeaderLen);
				unsigned char *pcData = (unsigned char*)byteBuffer.getPtr() + 
					iPacketHeaderLen;

				packet->setPacketType(tMediaFramePacketHeader.ePacketType);

				if (!packet->setFrameData(pcData,
					tMediaFramePacketHeader.nPacketSize,
					tMediaFramePacketHeader.lTimeStamp))
				{
					logPrint(MX_LOG_ERROR, "CAudioSourceInputServer setFrameData error");
					return;
				}

				if (!pushFrameData(packet))
				{
					std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
					popFrameData(lostPacket);
					m_iLostFrameNum++;
					//logPrint(MX_LOG_ERROR, "lost frame num: %d", m_iLostFrameNum);
					if (!pushFrameData(packet))
					{
						logPrint(MX_LOG_ERROR, "CAudioSourceInputServer insert frame error");
					}
				}
			}
		}
	}

	mxbool CAudioSourceInputServer::open()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "openAudio");

		cJSON_AddStringToObject(jsonParam, "moduleGUID", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "moduleName", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "key", "");
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

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(m_strAudioManageGUID,
				m_strAudioMangeRemoteEventServer,
				(unsigned char*)strOpen.c_str(), strOpen.length());

			std::cout << strResult << std::endl;
		}

		return mxtrue;
	}

	mxbool CAudioSourceInputServer::close()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "closeAudio");

		cJSON_AddStringToObject(jsonParam, "moduleGUID", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "moduleName", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "serverName", m_strChannelName.c_str());
		cJSON_AddStringToObject(jsonParam, "channelID", m_strChannelName.c_str());

		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strClose = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(m_strAudioManageGUID,
				m_strAudioMangeRemoteEventServer,
				(unsigned char*)strClose.c_str(), strClose.length());

			std::cout << strResult << std::endl;
		}

		return mxtrue;
	}

	mxbool CAudioSourceInputServer::config(std::string strConfig)
	{
		return mxbool();
	}

	mxbool CAudioSourceInputServer::reset()
	{
		return mxbool();
	}

	void CAudioSourceInputServer::updatePacketTime()
	{
		m_iPacketReceiveTime = time(NULL);
	}

	mxbool CAudioSourceInputServer::noPacket()
	{
		int iNow = time(NULL);

		if ((iNow - m_iPacketReceiveTime) > 5)
			return mxtrue;

		return mxfalse;
	}

	bool CAudioSourceInputServer::pushFrameData(
		std::shared_ptr<CMediaFramePacket> &packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CAudioSourceInputServer::popFrameData(
		std::shared_ptr<CMediaFramePacket> &packet)
	{
		m_objPacketQueue.pop(packet);
	}

	mxbool CAudioSourceInputServer::initAudioManageRemoteEventServer()
	{
		if (!m_module->getConfig("AUDIO_MANAGE_REMOTE_EVENT",
			"GUID", m_strAudioManageGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("AUDIO_MANAGE_REMOTE_EVENT",
			"SERVER", m_strAudioMangeRemoteEventServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}
}
