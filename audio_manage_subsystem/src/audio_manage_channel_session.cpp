#include "audio_manage_channel_session.h"

namespace maix {
	CAudioManageChannelSession::CAudioManageChannelSession(
		CAudioSourceInputServer * objAudioSourceInputServer)
		: m_objAudioSourceInputServer(objAudioSourceInputServer)
	{
		m_eState = E_A_OPENING;
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
				if (m_objAudioSourceInputServer && m_objAudioSourceInputServer->open())
				{
					m_eState = E_A_OPENED;
					m_objAudioSourceInputServer->updatePacketTime();
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
				if (m_objAudioSourceInputServer && m_objAudioSourceInputServer->noPacket())
				{
					m_eState = E_A_OPENING;
				}
				break;
			}
			case E_A_CLOSEING:
			{
				if (m_objAudioSourceInputServer && m_objAudioSourceInputServer->close())
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
			Sleep(1000);
#else
			sleep(1);
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
