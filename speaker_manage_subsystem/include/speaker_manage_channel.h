#ifndef __SPEAKER_MANAGE_CHANNEL_H__
#define __SPEAKER_MANAGE_CHANNEL_H__
#include "module.h"
#include "speaker_source_input_server.h"
#include <list>
#include "media_interface.h"

namespace maix {

	class MAIX_EXPORT CSpeakerManageChannel
	{
	public:
		CSpeakerManageChannel(CModule * module, std::string strName,
			CSpeakerSourceInputServer* objSpeakerSourceInputServer);
		~CSpeakerManageChannel();

		mxbool init();
		mxbool unInit();
		mxbool open();
		mxbool close();
		mxbool config(std::string strConfig);
		mxbool reset();

		void run();
	private:
		CModule * m_module;
		std::string m_strName;
		CSpeakerSourceInputServer* m_objSpeakerSourceInputServer;

		std::string m_strModuleGUID;
		std::string m_strModuleName;

		std::string m_strSpeakerSourceGUID;
		std::string m_strSpeakerSourceServer;

		std::mutex m_mutexChannelConfig;
		std::list<T_ServerConfig> m_listChannelConfig;
	};
}
#endif //__SPEAKER_MANAGE_CHANNEL_H__
