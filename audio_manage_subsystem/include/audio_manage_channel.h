#ifndef __AUDIO_MANAGE_CHANNEL_H__
#define __AUDIO_MANAGE_CHANNEL_H__
#include "module.h"
#include "audio_codec_entity.h"
#include "audio_source_input_server.h"
#include <list>

namespace maix {

	class MAIX_EXPORT CAudioManageChannel
	{
	public:
		CAudioManageChannel(CModule * module, std::string strName,
			CAudioSourceInputServer* objAudioSourceInputServer);
		~CAudioManageChannel();

		mxbool init();
		mxbool unInit();
		mxbool open(std::string strGUID, std::string strServerName,
			std::string strKey);
		mxbool close(std::string strGUID, std::string strServerName);
		mxbool config(std::string strConfig);
		mxbool reset();
		void run();
	private:
		CModule * m_module;
		std::string m_strName;
		CAudioSourceInputServer* m_objAudioSourceInputServer;

		std::mutex m_mutexClient;
		std::map<std::string,
			std::map<std::string, std::string >> m_mapClient;

		std::shared_ptr <CAudioCodecEntity> m_objAudioCodec;
		unsigned char* m_pcEncryptData;
		int m_iEncryptDataLen;
	};
}
#endif //__AUDIO_MANAGE_CHANNEL_H__
