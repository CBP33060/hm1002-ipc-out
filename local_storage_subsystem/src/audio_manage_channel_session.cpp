#include "audio_manage_channel_session.h"

namespace maix {
	CAudioManageChannelSession::CAudioManageChannelSession(
		CAudioSourceInputServer* objVideoSourceInputServer)
		: m_objVideoSourceInputServer(objVideoSourceInputServer)
	{
		m_eState = E_A_IDLE;
	}

	CAudioManageChannelSession::~CAudioManageChannelSession()
	{
	}

	mxbool CAudioManageChannelSession::init()
	{
		return mxtrue;
	}

	mxbool CAudioManageChannelSession::unInit()
	{
		return mxbool();
	}

	void CAudioManageChannelSession::run()
	{
		while (1)
		{
			switch (m_eState)
			{
			case E_A_OPENING:
			{
				if (m_objVideoSourceInputServer && m_objVideoSourceInputServer->open())
				{
					m_eState = E_A_OPENED;
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
			case E_A_OPENED:
			{
				if (m_objVideoSourceInputServer && 
					m_objVideoSourceInputServer->noPacket())
				{
					m_eState = E_A_OPENING;
				}
				break;
			}
			case E_A_CLOSEING:
			{
				if (m_objVideoSourceInputServer && 
					m_objVideoSourceInputServer->close())
				{
					m_eState = E_A_CLOSEED;
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
			case E_A_CLOSEED:
			{
				break;
			}
			case E_A_IDLE:
			{

				break;
			}
			default:
				break;
			}

#ifdef WIN32
			Sleep(500);
#else
			usleep(500);
#endif
		}
	}

	E_A_STATE CAudioManageChannelSession::getState()
	{
		return m_eState;
	}

	mxbool CAudioManageChannelSession::setState(E_A_STATE eState)
	{
		m_eState = eState;
		return mxtrue;
	}
}
