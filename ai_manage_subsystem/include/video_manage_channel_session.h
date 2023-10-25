#ifndef __VIDEO_MANAGE_CHANNEL_SESSION_H__
#define __VIDEO_MANAGE_CHANNEL_SESSION_H__
#include "video_manage_channel.h"
#include <list>

namespace maix {
	typedef enum
	{
		E_OPENING,
		E_OPENED,
		E_CLOSEING,
		E_CLOSEED,
		E_IDLE,
	}E_STATE;

	class MAIX_EXPORT CVideoManageChannelSession
	{
	public:
		CVideoManageChannelSession(
			CVideoSourceInputServer * objVideoSourceInputServer);
		~CVideoManageChannelSession();

		mxbool init();
		mxbool unInit();
	
		void run();

		E_STATE getState();
		mxbool setState(E_STATE eState);

	private:
		CVideoSourceInputServer * m_objVideoSourceInputServer;
		E_STATE m_eState;
	};
}
#endif //__VIDEO_MANAGE_CHANNEL_SESSION_H__
