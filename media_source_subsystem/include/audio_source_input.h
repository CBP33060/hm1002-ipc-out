#ifndef __AUDIO_SOURCE_INPUT_H__
#define __AUDIO_SOURCE_INPUT_H__
#include "module.h"
#include "media_interface.h"
#include "audio_source_channel.h"

namespace maix {
	class MAIX_EXPORT CAudioSourceInput
	{
	public:
		CAudioSourceInput(int iChnSN, std::string strName, E_P_TYPE ePacketType,
			CModule* module, std::shared_ptr<CMediaInterface> objInterface);
		~CAudioSourceInput();

		mxbool init(std::shared_ptr<CAudioSourceChannel> channel);
		mxbool unInit();
		void run();
		int64_t getCurrentTime();

	private:
		std::string m_strName;
		int m_iChnSN;
		E_P_TYPE m_ePacketType;
		CModule* m_module;
		std::shared_ptr<CMediaInterface> m_interface;
		std::shared_ptr<CAudioSourceChannel> m_channel;
		int m_iLostFrameNum;
	};
}
#endif //__AUDIO_SOURCE_INPUT_H__
