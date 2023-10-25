#ifndef __AUDIO_MANAGE_CHANNEL_H__
#define __AUDIO_MANAGE_CHANNEL_H__
#include "module.h"
#include "audio_source_input_server.h"
#include <list>

namespace maix {

	class MAIX_EXPORT CAudioManageChannel
	{
	public:
		CAudioManageChannel(CModule * module, std::string strName,
			CAudioSourceInputServer* objVideoSourceInputServer);
		~CAudioManageChannel();

		mxbool init();
		mxbool unInit();
		void run();

	private:
		CModule * m_module;
		std::string m_strName;
		CAudioSourceInputServer* m_objAudioSourceInputServer;
	};
}
#endif //__AUDIO_MANAGE_CHANNEL_H__
