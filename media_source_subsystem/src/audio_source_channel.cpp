#include "audio_source_channel.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include "log_mx.h"

namespace maix {
	CAudioSourceChannel::CAudioSourceChannel(std::string strName, 
		CModule * module)
		: m_module(module)
	{
		m_strName = strName;
	}

	CAudioSourceChannel::~CAudioSourceChannel()
	{
	}

	mxbool CAudioSourceChannel::init()
	{
		if (!m_objPacketQueue.init(60, 0))
			return mxfalse;

		return mxtrue;
	}

	mxbool CAudioSourceChannel::unInit()
	{
		return mxfalse;
	}

	void CAudioSourceChannel::run()
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
			for (iter = m_mapClient.begin(); iter != m_mapClient.end();)
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

	mxbool CAudioSourceChannel::open(std::string strGUID, std::string strServerName)
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

	mxbool CAudioSourceChannel::close(std::string strGUID, std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		if (m_mapClient.count(strGUID) != 0)
			m_mapClient[strGUID].remove(strServerName);
		return mxtrue;
	}

	mxbool CAudioSourceChannel::config(std::string strConfig)
	{
		return mxbool();
	}

	mxbool CAudioSourceChannel::reset()
	{
		return mxbool();
	}

	bool CAudioSourceChannel::pushFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CAudioSourceChannel::popFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		m_objPacketQueue.pop(packet);
	}

}
