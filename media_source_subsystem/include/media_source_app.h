#ifndef __MEDIA_SOURCE_APP__
#define __MEDIA_SOURCE_APP__
#include "app.h"

namespace maix {
	class CMediaSourceApp : public CApp
	{
	public:
		CMediaSourceApp(std::string strName);
		~CMediaSourceApp();

		mxbool init();
		mxbool unInit();
		mxbool initVideoSource(std::string strConfigPath);
		mxbool unInitVideoSource();
		mxbool initAudioSource(std::string strConfigPath);
		mxbool unInitAudioSource();
		mxbool initSpeakerSource(std::string strConfigPath);
		mxbool unInitSpeakerSource();
		mxbool initMcuSerialPort(std::string strConfigPath);
		mxbool unInitMcuSerialPort();

	private:
		std::string m_strVideoModuleName;
		std::string m_strVideoGUID;
		std::string m_strAudioModuleName;
		std::string m_strAudioGUID;
		std::string m_strSpeakerModuleName;
		std::string m_strSpeakerGUID;
		std::string m_strMCUSerialPortModuleName;
		std::string m_strMCUSerialPortGUID;
	};

}
#endif //__MEDIA_SOURCE_APP__
