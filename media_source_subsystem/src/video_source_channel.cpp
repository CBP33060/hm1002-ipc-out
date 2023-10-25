#include "video_source_channel.h"
#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#endif
#include <list>
#include <iostream>
#include "log_mx.h"
#include "cJSON.h"
#include "video_source_module.h"

namespace maix {
	CVideoSourceChannel::CVideoSourceChannel(std::string strName, E_P_TYPE ePacketType,
		CModule* module,void *objVideoSourceModule)
		: m_module(module)
	{
		m_strName = strName;
		m_ePacketType = ePacketType;
		m_objVideoSourceModule =objVideoSourceModule;

		m_jpeg = mxtrue;
		m_iClientNumCount = 0;
	}

	CVideoSourceChannel::~CVideoSourceChannel()
	{
	}

	mxbool CVideoSourceChannel::init()
	{
		if(m_ePacketType == E_P_VIDEO_YUV)
		{
			if (!m_objPacketQueue.init(3, 0))
				return mxfalse;
		}
		else
		{
			if (!m_objPacketQueue.init(90, 0))
				return mxfalse;
		}

		return mxtrue;
	}

	mxbool CVideoSourceChannel::unInit()
	{
		return mxfalse;
	}

	void CVideoSourceChannel::run()
	{
		while (1)
		{
			if (m_mapClient.size() == 0)
			{
#ifdef	WIN32
				Sleep(500);
#else
				usleep(1000 * 500);
#endif
				continue;
			}

			std::shared_ptr<CMediaFramePacket> packet = NULL;

			if(m_jpeg)
			{
				getJpegFrameData(packet);
				m_jpeg = mxfalse;
			}
			else
				popFrameData(packet);

			if (!packet)
			{
// #ifdef	WIN32
// 				Sleep(500);
// #else
// 				usleep(1000 * 500);
// #endif	
				continue;
			}

			std::unique_lock<std::mutex> lock(m_mutexClient);
			std::map<std::string, std::list<std::string>>::iterator  iter;
			for (iter = m_mapClient.begin(); iter != m_mapClient.end(); )
			{
				std::string strGUID = iter->first;
				std::list<std::string> listServer = iter->second;

				std::list<std::string>::iterator iterList;
				for (iterList = listServer.begin(); 
					iterList != listServer.end(); iterList++)
				{
					// logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
					// 	m_strName.c_str(), packet->getFrameDataLen());

					std::string strResult = m_module->output(strGUID, 
						*iterList, packet->getFrameData(),
						packet->getFrameDataLen());
					if (strResult.compare("disconnect") == 0)
					{
						m_mapClient[strGUID].remove(*iterList);
					}
				}

				if (m_mapClient[strGUID].size() == 0)
				{
					m_mapClient.erase(iter++);
					continue;
				}
				iter++;
			}
		}
	}

	mxbool CVideoSourceChannel::open(std::string strGUID, std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		if (m_mapClient.count(strGUID) == 0)
			m_mapClient[strGUID].push_back(strServerName);
		else
		{
			m_mapClient[strGUID].remove(strServerName);
			m_mapClient[strGUID].push_back(strServerName);
		}
		

		return mxtrue;
	}

	mxbool CVideoSourceChannel::close(std::string strGUID, std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		if (m_mapClient.count(strGUID) != 0)
			m_mapClient[strGUID].remove(strServerName);


		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonParam, "interfaceName", "CLoadZeratulVideoInterface");
		cJSON_AddStringToObject(pJsonParam, "configName", "SetNightShot");

		cJSON *jsonValue = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonValue, "value", "4");

		cJSON_AddItemToObject(pJsonParam, "configValue", jsonValue);

        char *out = cJSON_PrintUnformatted(pJsonParam);

		CVideoSourceModule *objVideoSourceModule = (CVideoSourceModule *)m_objVideoSourceModule;
		objVideoSourceModule->remoteEventServerProc("configVideo",out);

        if (pJsonParam)
        {
            cJSON_Delete(pJsonParam);
            pJsonParam = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }

		return mxtrue;
	}

	mxbool CVideoSourceChannel::config(std::string strConfig)
	{
		return mxbool();
	}

	mxbool CVideoSourceChannel::reset()
	{
		return mxbool();
	}

    int CVideoSourceChannel::getMapClientNum()
    {
        int result = 0;
		m_iClientNumCount++;
        result = m_mapClient.size();
		if(m_iClientNumCount == 300)
		{
			logPrint(MX_LOG_INFOR,"getMapClientNum = %d",result);
			m_iClientNumCount = 0;
		}
        return result;
    }

	bool CVideoSourceChannel::pushFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CVideoSourceChannel::popFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		m_objPacketQueue.pop(packet);
	}

    void CVideoSourceChannel::handleEventJpeg()
    {
        logPrint(MX_LOG_ERROR,"handleEventJpeg");
        m_jpeg = true;
    }

	mxbool CVideoSourceChannel::getJpegFrameData(std::shared_ptr<CMediaFramePacket> &packet)
	{
        logPrint(MX_LOG_ERROR,"getJpegFrameData");
		FILE * fpJpegFile = fopen("/tmp/cover.jpeg", "rb");
		if (fpJpegFile == NULL) {
			logPrint(MX_LOG_ERROR,"fopen /tmp/cover.jpeg error!");
			return mxfalse;
		}

		fseek(fpJpegFile,0,SEEK_END);
		long filesize = ftell(fpJpegFile);
		fseek(fpJpegFile,0,SEEK_SET);

		char * framedata = (char *)malloc(filesize);
		if(framedata == NULL)
		{
			logPrint(MX_LOG_ERROR,"malloc jpeg filesize %d error!", filesize);
			fclose(fpJpegFile);
			return mxfalse;
		}
		memset(framedata,0,filesize);

		char buf[1024] = { 0 };
		int size = 0, dataLen = 0;
		while(1) {
			memset(buf,0,sizeof(buf));
			size = fread(buf, 1, sizeof(buf), fpJpegFile);
			if (size > 0) {
				memcpy(framedata + dataLen, buf, size);
				dataLen += size;
			} else {
				break;
			}
		}
		fclose(fpJpegFile);

		packet = std::shared_ptr<CMediaFramePacket>(new CMediaFramePacket());
		packet->setPacketType(E_P_VIDEO_JPEG);
		if (!packet->setFrameData((unsigned char*)framedata, filesize, getCurrentTime(), 0))
		{
			logPrint(MX_LOG_ERROR,"packet setFrameData jpeg error!");
			free(framedata);
			return mxfalse;
		}

		free(framedata);
		return mxtrue;
	}

	int64_t CVideoSourceChannel::getCurrentTime()
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
