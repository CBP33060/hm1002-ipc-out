#include "audio_source_input_server.h"
#include <sys/timeb.h>
#include <iostream>
#include "media_frame_packet.h"
#include "cJSON.h"
#include "log_mx.h"

namespace maix {
	CAudioSourceInputServer::CAudioSourceInputServer(CModule * module)
		: m_module(module)
	{
		m_iPacketReceiveTime = 0;
		m_iLostFrameNum = 0;
	}

	CAudioSourceInputServer::~CAudioSourceInputServer()
	{
	}

	mxbool CAudioSourceInputServer::init(std::string strChannelName,
		int iType,
		std::string strIP,
		int iPort,
		std::string strUnix,
		int iLen)
	{
		if (!m_objPacketQueue.init(60, 500))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "GUID", m_strModuleGUID))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "NAME", m_strModuleName))
			return mxfalse;

		if (!initAudioSourceRemoteEventServer())
			return mxfalse;

		m_strChannelName = strChannelName;
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

	mxbool CAudioSourceInputServer::initAudioSourceRemoteEventServer()
	{
		if (!m_module->getConfig("AUDIO_SOURCE_REMOTE_EVENT",
			"GUID", m_strAudioSourceGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("AUDIO_SOURCE_REMOTE_EVENT",
			"SERVER", m_strAudioSourceRemoteEventServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CAudioSourceInputServer::open()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "openAudio");

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

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(m_strAudioSourceGUID,
				m_strAudioSourceRemoteEventServer,
				(unsigned char*)strOpen.c_str(), strOpen.length());

			if (strResult.length() > 0)
			{
				std::cout << strResult << std::endl;
				std::string strCode;
				std::string strMsg;
				std::string strErr;

				if (parseResult(strResult, strCode, strMsg, strErr))
				{
					if (strCode.compare("200") != 0)
					{
						return mxfalse;
					}
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				return mxfalse;
			}
		}
		else
		{
			return mxfalse;
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
			std::string strResult = m_module->output(m_strAudioSourceGUID,
				m_strAudioSourceRemoteEventServer,
				(unsigned char*)strClose.c_str(), strClose.length());

			if (strResult.length() > 0)
			{
				std::cout << strResult << std::endl;
				std::string strCode;
				std::string strMsg;
				std::string strErr;

				if (parseResult(strResult, strCode, strMsg, strErr))
				{
					if (strCode.compare("200") != 0)
					{
						return mxfalse;
					}
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				return mxfalse;
			}
		}
		else
		{
			return mxfalse;
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
				memcpy(&tMediaFramePacketHeader, byteBuffer.getPtr(), iPacketHeaderLen);
				char *pcData = byteBuffer.getPtr() + iPacketHeaderLen;

				packet->setPacketType(tMediaFramePacketHeader.ePacketType);

				if (!packet->setFrameData((unsigned char*)pcData,
					tMediaFramePacketHeader.nPacketSize, 
					tMediaFramePacketHeader.lTimeStamp,
					tMediaFramePacketHeader.iFrameSeq))
					return;

				if (!pushFrameData(packet))
				{
					std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
					popFrameData(lostPacket);
					m_iLostFrameNum++;
					//logPrint(MX_LOG_ERROR, "audio channel %s lost frame num: %d",
					//	m_strChannelName.c_str(), m_iLostFrameNum);

					if (!pushFrameData(packet))
					{
						logPrint(MX_LOG_DEBUG, "channel %s insert frame error",
							m_strChannelName.c_str());
					}
				}
			}
		}
	}

	void CAudioSourceInputServer::frameProc(unsigned char* byteBuffer, int len)
	{
		updatePacketTime();
		int iDataSize = len;
		if (iDataSize > 0)
		{
			std::shared_ptr<CMediaFramePacket>  packet(
				new CMediaFramePacket());
			int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
			if (iDataSize > iPacketHeaderLen)
			{
				T_MediaFramePacketHeader tMediaFramePacketHeader;
				memcpy(&tMediaFramePacketHeader, byteBuffer, iPacketHeaderLen);
				unsigned char *pcData = byteBuffer + iPacketHeaderLen;

				packet->setPacketType(tMediaFramePacketHeader.ePacketType);

				if (!packet->setFrameData(pcData,
					tMediaFramePacketHeader.nPacketSize, 
					tMediaFramePacketHeader.lTimeStamp,
					tMediaFramePacketHeader.iFrameSeq))
					return;

				if (!pushFrameData(packet))
				{
					std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
					popFrameData(lostPacket);
					m_iLostFrameNum++;
					//logPrint(MX_LOG_ERROR, "audio channel %s lost frame num: %d",
					//	m_strChannelName.c_str(), m_iLostFrameNum);

					if (!pushFrameData(packet))
					{
						logPrint(MX_LOG_DEBUG, "channel %s insert frame error",
							m_strChannelName.c_str());
					}
				}
			}
		}
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

	mxbool CAudioSourceInputServer::parseResult(std::string & strInput, 
		std::string & code, std::string & strMsg, std::string & strErr)
	{
		if (strInput.empty())
			return mxfalse;

		cJSON *jsonRoot = cJSON_Parse(strInput.c_str());

		if (jsonRoot)
		{
			cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
			if (jsonCode)
			{
				code = std::string(jsonCode->valuestring);
			}
			else
			{
				return mxfalse;
			}

			cJSON *jsonMsg = cJSON_GetObjectItem(jsonRoot, "msg");
			if (jsonMsg)
			{
				char *pcMsg = cJSON_Print(jsonMsg);
				if (pcMsg)
				{
					strMsg = std::string(pcMsg);
					free(pcMsg);
				}
			}

			cJSON *jsonErr = cJSON_GetObjectItem(jsonRoot, "errMsg");
			if (jsonErr)
			{
				strErr = std::string(jsonErr->valuestring);
			}
			else
			{
				return mxfalse;
			}

			cJSON_Delete(jsonRoot);
		}

		return mxtrue;
	}
}
