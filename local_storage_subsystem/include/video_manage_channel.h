#ifndef __VIDEO_MANAGE_CHANNEL_H__
#define __VIDEO_MANAGE_CHANNEL_H__
#include "module.h"
#include "video_source_input_server.h"
#include <list>

namespace maix {

	class MAIX_EXPORT CVideoManageChannel
	{
	public:
		CVideoManageChannel(CModule * module, std::string strName,
			CVideoSourceInputServer* objVideoSourceInputServer);
		~CVideoManageChannel();

		mxbool init();
		mxbool unInit();

		void run();
	private:
		CModule * m_module;
		std::string m_strName;
		std::string m_strKey;
		CVideoSourceInputServer* m_objVideoSourceInputServer;
	};
}
#endif //__VIDEO_MANAGE_CHANNEL_H__