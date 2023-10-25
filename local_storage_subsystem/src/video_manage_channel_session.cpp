#include "video_manage_channel_session.h"

namespace maix {
	CVideoManageChannelSession::CVideoManageChannelSession(
		CVideoSourceInputServer * objVideoSourceInputServer)
		: m_objVideoSourceInputServer(objVideoSourceInputServer)
	{
		m_eState = E_IDLE;
	}

	CVideoManageChannelSession::~CVideoManageChannelSession()
	{
	}

	mxbool CVideoManageChannelSession::init()
	{
		return mxtrue;
	}

	mxbool CVideoManageChannelSession::unInit()
	{
		return mxbool();
	}

	void CVideoManageChannelSession::run()
	{
		while (1)
		{
			switch (m_eState)
			{
			case E_OPENING:
			{
				if (m_objVideoSourceInputServer && 
					m_objVideoSourceInputServer->open())
				{
					m_eState = E_OPENED;
					m_objVideoSourceInputServer->updatePacketTime();
				}
				else
				{
#ifdef _WIN32
					Sleep(1000);
#else
					sleep(1);
#endif
					continue;
				}
				break;
			}
			case E_OPENED:
			{
				if (m_objVideoSourceInputServer && 
					m_objVideoSourceInputServer->noPacket())
				{
					m_eState = E_OPENING;
				}
				break;
			}
			case E_CLOSEING:
			{
				if (m_objVideoSourceInputServer && 
					m_objVideoSourceInputServer->close())
				{
					m_eState = E_CLOSEED;
				}
				else
				{
#ifdef _WIN32
					Sleep(1000);
#else
					sleep(1);
#endif
					continue;
				}
				break;
			}
			case E_CLOSEED:
			{
				break;
			}
			case E_IDLE:
			{

				break;
			}
			default:
				break;
			}

#ifdef WIN32
			Sleep(500);
#else
			usleep(500*1000);
#endif
		}
	}

	E_STATE CVideoManageChannelSession::getState()
	{
		return m_eState;
	}

	mxbool CVideoManageChannelSession::setState(E_STATE eState)
	{
		m_eState = eState;
		return mxtrue;
	}
}