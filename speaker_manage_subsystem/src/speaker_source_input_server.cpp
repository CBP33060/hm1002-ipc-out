#include "speaker_source_input_server.h"
#include <iostream>
#include "media_frame_packet.h"
#include "cJSON.h"
#include "log_mx.h"
#include "common.h"
#include "crypt_api_mx.h"
#include "common.h"
#define DECRYPT_DATA_LEN (5*1024)

namespace maix {
	CSpeakerSourceInputServer::CSpeakerSourceInputServer(CModule * module)
		: m_module(module)
		, m_iLostFrameNum(0)
		, m_pcDecryptData(NULL)
		, m_iDecryptDataLen(0)
	{
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
		if (!m_objPacketQueue.init(30, 30))
			return mxfalse;


		if (!m_module->getConfig("MODULE", "GUID", m_strModuleGUID))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "NAME", m_strModuleName))
			return mxfalse;

		m_strChannelName = strChannelName;
		m_iType = iType;
		m_strIP = strIP;
        if ((m_strIP.compare("0.0.0.0") == 0) || (m_strIP.find("192.168.10.") != 0))
		{
			getLocalIPByName(ETH0, m_strIP);
		}
		m_iPort = iPort;
		m_strUnix = strUnix;
		m_iLen = iLen;

		// if (!getKey(m_strKey))
		// 	return mxfalse;
        m_strKey = getPSK();

		if (!m_pcDecryptData)
		{
			m_pcDecryptData = (unsigned char*)malloc(DECRYPT_DATA_LEN);
			if (!m_pcDecryptData)
				return mxfalse;
		}

		m_iDecryptDataLen = DECRYPT_DATA_LEN;

		return mxtrue;
	}

	mxbool CSpeakerSourceInputServer::unInit()
	{
		return mxbool();
	}

	void CSpeakerSourceInputServer::frameProc(RCF::ByteBuffer byteBuffer)
	{
		int iDataSize = byteBuffer.getLength();
		if (iDataSize > 0 && m_strKey.length() > 0)
		{
			RCF::TransportType type =
				RCF::getCurrentRcfSession().getTransportType();
			if (type == RCF::Tt_Tcp ||
				type == RCF::Tt_Udp)
			{
				int iDecryptDataLen = 0;
				if (iDataSize > DECRYPT_DATA_LEN)
				{
					free(m_pcDecryptData);
					m_pcDecryptData = NULL;
					m_pcDecryptData = (unsigned char*)malloc(
						iDataSize + 48);
					if (m_pcDecryptData)
					{
						m_iDecryptDataLen = iDataSize + 48;
					}
					else
					{
						logPrint(MX_LOG_ERROR, "malloc decrypt data buffer failed");
						return;
					}

				}
				else
				{
					if (!m_pcDecryptData)
					{
						m_pcDecryptData = (unsigned char*)malloc(DECRYPT_DATA_LEN);
						if (!m_pcDecryptData)
							return;

						m_iDecryptDataLen = DECRYPT_DATA_LEN;
					}

				}

				if (crypto_aes128_decrypt((unsigned char*)m_strKey.c_str(),
					(unsigned char*)byteBuffer.getPtr(),
					iDataSize,
					m_pcDecryptData, &iDecryptDataLen) != 0)
					return;

				std::shared_ptr<CMediaFramePacket>  packet(
					new CMediaFramePacket());
				int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
				if (iDataSize > iPacketHeaderLen)
				{
					T_MediaFramePacketHeader tMediaFramePacketHeader;
					memcpy(&tMediaFramePacketHeader, m_pcDecryptData, iPacketHeaderLen);
					unsigned char *pcData = m_pcDecryptData + iPacketHeaderLen;

					packet->setPacketType(tMediaFramePacketHeader.ePacketType);
                    packet->setReserve_1(tMediaFramePacketHeader.iReserve_1);
                    packet->setReserve_2(tMediaFramePacketHeader.iReserve_2);

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
						logPrint(MX_LOG_DEBUG, "channel %s lost frame num: %d",
							m_strChannelName.c_str(), m_iLostFrameNum);

						if (!pushFrameData(packet))
						{
							logPrint(MX_LOG_DEBUG, "channel %s insert frame error",
								m_strChannelName.c_str());
						}
					}
				}
			}
			else if (type == RCF::Tt_UnixNamedPipe ||
				type == RCF::Tt_Win32NamedPipe)
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

					if (!pushFrameData(packet))
					{
						std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
						popFrameData(lostPacket);
						m_iLostFrameNum++;
						logPrint(MX_LOG_DEBUG, "channel %s lost frame num: %d",
							m_strChannelName.c_str(), m_iLostFrameNum);

						if (!pushFrameData(packet))
						{
							logPrint(MX_LOG_DEBUG, "channel %s insert frame error",
								m_strChannelName.c_str());
						}
					}
				}
			}
			else
			{
				return;
			}
		}
	}

	bool CSpeakerSourceInputServer::pushFrameData(
		std::shared_ptr<CMediaFramePacket> &packet)
	{
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
		cJSON_AddStringToObject(jsonParam, "key", m_strKey.c_str());
		cJSON_AddStringToObject(jsonParam, "serverName", m_strChannelName.c_str());
		cJSON_AddStringToObject(jsonParam, "channelID", m_strChannelName.c_str());

		cJSON_AddNumberToObject(jsonParam, "type", m_iType);
        if ((m_strIP.compare("0.0.0.0") == 0) || (m_strIP.find("192.168.10.") != 0))
		{
			getLocalIPByName(ETH0, m_strIP);
		}
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
}
