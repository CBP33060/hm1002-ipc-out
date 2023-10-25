#ifndef __DEV_PLAY_VOICE_H__
#define __DEV_PLAY_VOICE_H__
#include "module.h"
#include "b_queue.h"
#include "media_frame_packet.h"
#include "dev_play_voice_file_read.h"

namespace maix {

    #define PARA_AUDIO_LANGUAGE              "language"

	class MAIX_EXPORT CDevPlayVoice
	{
	public:
		CDevPlayVoice(CModule * module);
		~CDevPlayVoice();

		mxbool init();
		mxbool unInit();
		mxbool openSpeaker();
		mxbool sendframe(unsigned char* pcData, int iLen,
			E_P_TYPE ePacketType,E_VOICE_PLAY_LEVEL eVoiceLevel);
		mxbool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void run();
		int64_t getCurrentTime();

        mxbool playWithFileId(std::string strFileId , int iLevel,int iPlayTime);
        mxbool playWithFilePath(std::string strFilePath , int iLevel,int iPlayTime);
        mxbool stopWithFileId(std::string strFileId);

	private:

        mxbool loadAudioConfig();
        mxbool loadAudioRes();
        mxbool getPowerUpEndState();


		CModule * m_module;
		mxbool m_bInit;
		int m_iLostFrameNum;
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		std::string m_strDevModuleName;
		std::string m_strDevModuleGUID;

		std::string m_strSpeakerManageModuleGUID;
		std::string m_strSpeakerManageRemoteEventServer;
		std::string m_strSpeakerManageChannelServer;
		std::string m_strMCUModuleGUID;
		std::string m_strMCURemoteEventServer;

		std::thread m_threadPlayVoice;
		std::string m_strKey;
        
        std::shared_ptr<CDevPlayVoiceFileRead> m_objPlayVoiceFileRead;

        std::string m_strAudioResConfig;
        std::string m_strAudioResPath;
        // std::string m_strLanguage;
        std::map<std::string,std::string> m_mapAudioRes;
	};
}
#endif //__DEV_PLAY_VOICE_H__
