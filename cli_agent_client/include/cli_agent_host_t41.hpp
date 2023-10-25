#ifndef __CLI_AGENT_HOST_t41__
#define __CLI_AGENT_HOST_t41__

#include "zeratul_audio_interface.h"
#include "zeratul_speaker_interface.h"

namespace maix {
    class CCliAgentHostT41: public CCliAgentHost
    {
    public:
        CCliAgentHostT41();
        ~CCliAgentHostT41();

    public:
        virtual int AudioRecord(std::string strPath,int iFrameNum);
        virtual int AudioAplay(std::string strPath);
        virtual int LedSet(int iIndex,int iFlag); // iIndex:0 白光，1 红外;iFlag:0 关，1 开
        virtual int IrCutSet(int iFlag);
        virtual int NightModeSet(int iFlag);
        virtual std::string GetVersion();
        virtual std::string GetAlsValue();
        virtual std::string GetAlsRaw();
        virtual std::string TestAls();
        virtual std::string CalibrationAls();
    private:
        CZeratulAudioInterface *m_cAiInterface;
        CZeratulSpeakerInterface *m_cAoInterface;
    };
}

#endif /* __CLI_AGENT_HOST_t41__ */