#ifndef __VIDEO_SOURCE_INPUT_H__
#define __VIDEO_SOURCE_INPUT_H__
#include "module.h"
#include "media_interface.h"
#include "video_source_channel.h"

namespace maix {
	class MAIX_EXPORT CVideoSourceInput
	{
	public:
		CVideoSourceInput(int iChnSN, std::string strName, E_P_TYPE ePacketType,
			CModule* module, std::shared_ptr<CMediaInterface> objInterface);
		~CVideoSourceInput();

		mxbool init(std::shared_ptr<CVideoSourceChannel> channel);
		mxbool unInit();
		void run();
		int64_t getCurrentTime();
        mxbool getIDRframe();
        mxbool getJpegFrameData();

	private:
		std::string m_strName;
		int m_iChnSN;
		E_P_TYPE m_ePacketType;
		CModule* m_module;
		std::shared_ptr<CMediaInterface> m_interface;
		std::shared_ptr<CVideoSourceChannel> m_channel;
		int m_iLostFrameNum;
	};
}
#endif //__VIDEO_SOURCE_INPUT_H__
