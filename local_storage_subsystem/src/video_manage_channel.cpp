#include "video_manage_channel.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "cJSON.h"
#include "common.h"
#include "local_storage_module.h"

namespace maix {
	CVideoManageChannel::CVideoManageChannel(CModule * module,
		std::string strName,
		CVideoSourceInputServer * objVideoSourceInputServer)
		: m_module(module)
		, m_strName(strName)
		, m_objVideoSourceInputServer(objVideoSourceInputServer)
	{

	}

	CVideoManageChannel::~CVideoManageChannel()
	{
	}

	mxbool CVideoManageChannel::init()
	{
		return mxtrue;
	}

	mxbool CVideoManageChannel::unInit()
	{
		return mxbool();
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
#endif
					continue;
				}

				//std::cout <<"video: " << packet->getFrameDataLen() << std::endl;
				CLocalStorageModule *objLocalStorageModule =
					(CLocalStorageModule *)m_module;
				objLocalStorageModule->pushFrameDataToEvent(packet);
				
			}
			else
			{
#ifdef	WIN32
				Sleep(1000);
#else
				usleep(1000 * 1000);
#endif
			}
		}
	}

}