#ifndef __SPEAKER_SOURCE_CHANNEL_H__
#define __SPEAKER_SOURCE_CHANNEL_H__
#include "module.h"
#include "media_interface.h"
#include <list>
#include "speaker_source_input_server.h"

namespace maix {
	class MAIX_EXPORT CSpeakerSourceChannel
	{
	public:
		CSpeakerSourceChannel(int iChnSN, std::string strName,
			E_P_TYPE ePacketType,
			CSpeakerSourceInputServer* objSourceInput, 
			std::shared_ptr<CMediaInterface> objInterface);
		~CSpeakerSourceChannel();

		mxbool init();
		mxbool unInit();
		void run();

	private:
		std::string m_strName;
		int m_iChnSN;
		E_P_TYPE m_ePacketType;
		std::shared_ptr<CMediaInterface> m_interface;
		CSpeakerSourceInputServer *m_objSourceInput;
	};
}
#endif //__SPEAKER_SOURCE_CHANNEL_H__
