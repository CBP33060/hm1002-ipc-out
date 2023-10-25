#include "power_up_sound.h"
#include "opus.h"
#include "log_mx.h"
#include "unistd.h"
#include "speaker_source_module.h"
#include "speaker_source_input_server.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "fw_env_para.h"

#define MX_DECODE_RATE 16000
#define MX_DECODE_BIT 16
#define MX_DECODE_CHANNEL 1
#define MX_DECODE_MAX_FRAME_SIZE 1280*2

namespace maix {

    CPowerUpSound::CPowerUpSound()
    {
        m_bIsRun = false;
        m_bIsPlayEnd = mxtrue;
    }

    CPowerUpSound::~CPowerUpSound()
    {
    }

    void CPowerUpSound::init(CMediaSourceApp* appModule)
    {
        std::lock_guard<std::mutex> lck(m_mutexSound);
        m_appModule = appModule;
    }
    
    void CPowerUpSound::startPlaySound()
    {
        std::lock_guard<std::mutex> lck(m_mutexSound);
        if(m_bIsRun)
        {
            return;
        }
        m_bIsRun = true;
        m_bIsPlayEnd = false;
        logPrint(MX_LOG_ERROR, "startPlaySound");
        int ret = 0;
        size_t nSize = 1024 * 600;
        ret = pthread_attr_init(&m_ptAttr); 
        if (ret != 0)
        {
            logPrint(MX_LOG_ERROR, "pthread_attr_init failed");
        }

        ret = pthread_attr_setstacksize(&m_ptAttr, nSize);
        if (ret != 0)
        {
            logPrint(MX_LOG_ERROR, "pthread_attr_getstacksize failed");
        }

        pthread_create(&m_ptID, &m_ptAttr, threadRun, this);
    }

    void * CPowerUpSound::threadRun(void * pHandle)
    {
        CPowerUpSound* thread = (CPowerUpSound*) pHandle;
        thread->Routine();
        return NULL;
    }

    void CPowerUpSound::Routine()
    {
        while(m_bIsRun)
        {
            if(m_appModule){
                std::string speakerModelName;
                if (!m_appModule->getConfig("SPEAKER_CONFIG", "NAME", speakerModelName))
                {
                    return ;
		        }
                CSpeakerSourceModule * objSpeakerModule = dynamic_cast<CSpeakerSourceModule *>(m_appModule->getModule(speakerModelName).get());
                if(!objSpeakerModule)
                {
                    usleep(10 * 1000);
                    continue;
                }
                CSpeakerSourceInputServer* objSpeakerInputServet = objSpeakerModule->getSpeakerSourceInputServer();
                if(!objSpeakerInputServet)
                {
                    usleep(10 * 1000);
                    continue;
                }
                std::string sourdResPath;
                if (!m_appModule->getConfig("POWER_UP_AUDIO_PATH", "PATH", sourdResPath))
                {
                    return ;
		        }
                if(!m_bIsPlayEnd)
                {
                    sendFrame(objSpeakerInputServet,sourdResPath);
                    m_bIsPlayEnd = mxtrue;
                }
                logPrint(MX_LOG_ERROR, "end play power up");
                break;
            }
            else
            {
               usleep(10 * 1000); 
            }

        }
    }

    mxbool CPowerUpSound::sendFrame(CSpeakerSourceInputServer* sourceInputServer ,std::string soundResPath)
    {
        int readBufSize = 640;
        unsigned char *inBuf = NULL;
        inBuf = (unsigned char *)malloc(readBufSize*sizeof(short));
        if (inBuf == NULL) {
            return mxfalse;
        }
        unsigned char *outBuf = NULL;
        outBuf = (unsigned char *)malloc(MX_DECODE_MAX_FRAME_SIZE*MX_DECODE_CHANNEL*sizeof(short));
        if (outBuf == NULL) {
            logPrint(MX_LOG_INFOR, "[ERROR] %s: malloc audio outBuf error\n", __func__);
            free(inBuf);           
            return mxfalse;
        }

        int err;
        OpusDecoder *dec = opus_decoder_create(MX_DECODE_RATE, MX_DECODE_CHANNEL, &err);
        if (err != OPUS_OK)
        {
            logPrint(MX_LOG_INFOR, "Cannot create decoder: %s\n", opus_strerror(err));
            free(inBuf);
            free(outBuf);
            return mxfalse;
        }

        FILE *play_file = fopen(soundResPath.c_str(), "rb");
        if (play_file == NULL) {
            logPrint(MX_LOG_INFOR, "open power_up file err\n");
            free(inBuf);
            free(outBuf);
            opus_decoder_destroy(dec);
            return mxfalse;
        }

        unsigned int decFinalRange;
        short *out = (short*)malloc(MX_DECODE_MAX_FRAME_SIZE*MX_DECODE_CHANNEL*sizeof(short));
        if (out == NULL) {
            logPrint(MX_LOG_INFOR, "[ERROR] %s: malloc audio out error\n", __func__);
            free(inBuf);
            free(outBuf);
            fclose(play_file);
            opus_decoder_destroy(dec);
            return mxfalse;
        }
        // unsigned int encFinalRange;
        size_t num_read;
        int decodeLen;
        mxbool needPlay = true;
        int resultLen = -1;
        while (needPlay)
        {
            unsigned char ch[4];
            num_read = fread(ch, 1, 4, play_file);
            if (num_read!=4)
            {
                needPlay = false;
                break;
            }
                
            decodeLen = char_to_int(ch);
            if (decodeLen>1280 || decodeLen<0)
            {
                needPlay = false;
                break;
            }
            num_read = fread(ch, 1, 4, play_file);
            if (num_read!=4)
            {
                needPlay = false;
                break;
            }  
            // encFinalRange = char_to_int(ch);
            num_read = fread(inBuf, 1, decodeLen, play_file);
            if (num_read!=(size_t)decodeLen)
            {
                needPlay = false;                
                break;
            }

            opus_int32 output_samples = opus_decode(dec, inBuf, decodeLen, out, MX_DECODE_MAX_FRAME_SIZE, 0);
            if (output_samples>0)
            {
                int i;
                for(i=0;i<(output_samples)*MX_DECODE_CHANNEL;i++)
                {
                    short s;
                    s=out[i];
                    outBuf[2*i]=s&0xFF;
                    outBuf[2*i+1]=(s>>8)&0xFF;
                }
                resultLen = output_samples * sizeof(short)*MX_DECODE_CHANNEL;
            } else {
                logPrint(MX_LOG_ERROR, "error decoding frame %d \n",output_samples);
            }
            opus_decoder_ctl(dec, OPUS_GET_FINAL_RANGE(&decFinalRange));

            if (resultLen > 0)
            {
                std::shared_ptr<CMediaFramePacket>  packet(
                    new CMediaFramePacket());
                packet->setPacketType(E_P_AUDIO_PCM);
                packet->setReserve_1(E_SOURCE_FILE);
                packet->setReserve_2(E_LEVEL_HIGHEST);
                if (!packet->setFrameData(outBuf, resultLen, getCurrentTime(), 0))

                    return mxfalse;

                if (!sourceInputServer->pushFrameData(packet))
                {
                    std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
                    sourceInputServer->popFrameData(lostPacket);

                    logPrint(MX_LOG_DEBUG, "power up lost frame num");

                    if (!sourceInputServer->pushFrameData(packet))
                    {
                        logPrint(MX_LOG_DEBUG, "power up insert frame error");
                    }
                }
            }

            usleep(19 * 1000);
        }

        
        free(inBuf);
        free(outBuf);
        fclose(play_file);
        free(out);
        opus_decoder_destroy(dec);
        return mxtrue;
    }

    mxbool CPowerUpSound::getPlayEndState()
    {
        return m_bIsPlayEnd;
    }

    int64_t CPowerUpSound::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

    uint32_t CPowerUpSound::char_to_int(unsigned char ch[4])
    {
        return ((uint32_t)ch[0]<<24) | ((uint32_t)ch[1]<<16)
            | ((uint32_t)ch[2]<< 8) |  (uint32_t)ch[3];
    }

}
