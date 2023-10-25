#ifndef __POWER_UP_SOUND_H__
#define __POWER_UP_SOUND_H__

#include <string>
#include "module.h"
#include "media_interface.h"
#include "media_source_app.h"
#include <pthread.h>
#include "speaker_source_input_server.h"

namespace maix {

    class  CPowerUpSound
    {
    public:
        ~CPowerUpSound();

        CPowerUpSound(const CPowerUpSound&) = delete;
        CPowerUpSound& operator = (const CPowerUpSound&) = delete;
        static CPowerUpSound& getInstance(){
            static CPowerUpSound instance;
            return instance;
        }

        void init(CMediaSourceApp* appModule);
        void startPlaySound();
        mxbool getPlayEndState();

    private:
        CPowerUpSound();
        uint32_t char_to_int(unsigned char ch[4]);
        mxbool sendFrame(CSpeakerSourceInputServer* sourceInputServer,std::string soundResPath);
        int64_t getCurrentTime();

        static void * threadRun(void * pHandle);
        void Routine();

        mutable std::mutex m_mutexSound;
        CMediaSourceApp* m_appModule;
        pthread_attr_t m_ptAttr; 
	    pthread_t m_ptID;
        mxbool m_bIsRun;
        mxbool m_bIsPlayEnd;
    };
}
#endif //__POWER_UP_SOUND_H__
