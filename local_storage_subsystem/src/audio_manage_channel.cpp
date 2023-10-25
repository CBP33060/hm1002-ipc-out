#include "audio_manage_channel.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "cJSON.h"
#include <iostream>
#include "common.h"
#include "log_mx.h"
#include "local_storage_module.h"

namespace maix {
	CAudioManageChannel::CAudioManageChannel(CModule * module,
		std::string strName,
		CAudioSourceInputServer * objAudioSourceInputServer)
		: m_module(module)
		, m_strName(strName)
		, m_objAudioSourceInputServer(objAudioSourceInputServer)
	{
	
	}

	CAudioManageChannel::~CAudioManageChannel()
	{
	}

	mxbool CAudioManageChannel::init()
	{
		return mxtrue;
	}

	mxbool CAudioManageChannel::unInit()
	{
		return mxbool();
	}

	void CAudioManageChannel::run()
	{
		while (1)
		{
			if (m_objAudioSourceInputServer)
			{
				std::shared_ptr<CMediaFramePacket> packet = NULL;
				m_objAudioSourceInputServer->popFrameData(packet);

				if (!packet)
				{
#ifdef	WIN32
					Sleep(500);
#else
					usleep(500 * 1000);
#endif
					continue;
				}

				CLocalStorageModule *objLocalStorageModule = 
					(CLocalStorageModule *)m_module;
				objLocalStorageModule->pushFrameDataToEvent(packet);
				//logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
				//	m_strName.c_str(), packet->getFrameDataLen());
				
			}
			else
			{
#ifdef _WIN32
				Sleep(1000);
#else
				usleep(1000 * 1000);
#endif
			}
		}
	}

}
