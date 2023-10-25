#ifndef __CLI_AGENT_HOST__
#define __CLI_AGENT_HOST__

#include <string>
#include <memory>

namespace maix {
    class CCliAgentHost
    {
    public:
        CCliAgentHost();
        ~CCliAgentHost();

    public:
        virtual int FactoryReset();
        virtual int BurnInStart();
        virtual int BurnInStop();
        virtual int AudioRecord(std::string strPath,int iFrameNum);
        virtual int AudioAplay(std::string strPath);
        virtual std::string GetAlsValue();
        virtual int LedSet(int iIndex,int iFlag); // iIndex:0 白光，1 红外;iFlag:0 关，1 开
        virtual int IrCutSet(int iFlag); // iFlag:0 白天，1 夜晚
        virtual int NightModeSet(int iFlag); // iFlag:0 白天，1 夜晚
        virtual std::string GetVersion();
        virtual std::string GetSdcardInfo();
        virtual std::string TestAls();
        virtual std::string CalibrationAls();
        virtual std::string GetAlsRaw();
    private:

    };

    class CCliAgentHostFactory
    {
    public:
        static CCliAgentHost *GetInstance(std::string type);

    private:
        CCliAgentHostFactory();
        ~CCliAgentHostFactory();

    private:
        static CCliAgentHost *m_instance;
    };
}

#endif /* __CLI_AGENT_HOST__ */