#ifndef __DEV_PLAY_VOICE_FILE_READ_H__
#define __DEV_PLAY_VOICE_FILE_READ_H__

#include "module.h"
#include "b_queue.h"
#include "media_frame_packet.h"

namespace maix {

    typedef struct VOICE_PLAY_INFO
    {
        E_VOICE_PLAY_LEVEL  m_eLevel;
        std::string         m_strFilePath;      //播放路径
        int                 m_iPlayTime;        //需要播放时长      s
        long                m_lCurrentPlayTime; //当前播放时长   ms
    }T_VOICE_PLAY_INFO;

	class CDevPlayVoiceFileRead
	{
	public:
		CDevPlayVoiceFileRead();
		~CDevPlayVoiceFileRead();

        using PlayCallback = std::function<void(unsigned char* , int , E_P_TYPE ,E_VOICE_PLAY_LEVEL)>;
        using PowerUpCallback = std::function<mxbool()>;

        mxbool init();
        mxbool unInit();

        mxbool playWithFile(std::string strFilePath , int iLevel,int playTime);
        void registCallback(PlayCallback funCallback);
        mxbool stopWithFile(std::string strFilePath);
        void registPowerUpCallback(PowerUpCallback callback);

    private:

        bool playFileCompare(T_VOICE_PLAY_INFO first,T_VOICE_PLAY_INFO second);
        E_VOICE_PLAY_LEVEL getPlayLevel(int level);
        void run();
        void playWithFilePCM(std::string strPath);
        void playWithFileOPUS(T_VOICE_PLAY_INFO playInfo);
        uint32_t char_to_int(unsigned char ch[4]);

        mxbool m_bInit;
        mxbool m_bRun;

		std::thread m_threadPlay;
		std::mutex m_mutexPlay;
		std::condition_variable m_conditionPlay;

        std::deque<T_VOICE_PLAY_INFO> m_dqPlayFileList;             //文件播放列表

        PlayCallback m_funCallback;
        PowerUpCallback m_powerUpCallback;

        mxbool m_bPowerUpPlayState;
        std::string m_strStopPath;

    };
}
#endif //__DEV_PLAY_VOICE_FILE_READ_H__
