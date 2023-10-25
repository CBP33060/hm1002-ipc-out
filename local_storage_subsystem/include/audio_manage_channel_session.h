#ifndef __AUDIO_MANAGE_CHANNEL_SESSION_H__
#define __AUDIO_MANAGE_CHANNEL_SESSION_H__
#include "audio_source_input_server.h"
#include <list>

namespace maix {
	typedef enum
	{
		E_A_OPENING,
		E_A_OPENED,
		E_A_CLOSEING,
		E_A_CLOSEED,
		E_A_IDLE,
	}E_A_STATE;

	class MAIX_EXPORT CAudioManageChannelSession
	{
	public:
		CAudioManageChannelSession(
			CAudioSourceInputServer* objVideoSourceInputServer);
		~CAudioManageChannelSession();

		mxbool init();
		mxbool unInit();

		void run();

		E_A_STATE getState();
		mxbool setState(E_A_STATE eState);

	private:
		CAudioSourceInputServer* m_objVideoSourceInputServer;
		E_A_STATE m_eState;
	};
}
#endif //__AUDIO_MANAGE_CHANNEL_SESSION_H__
