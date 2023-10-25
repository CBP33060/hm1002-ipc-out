#ifndef __CLI_AGENT_MCU__
#define __CLI_AGENT_MCU__

#include <string>
#include <memory>

#include "cli_agent_mcu_io_device.hpp"

namespace maix {
    class CCliAgentMcu
    {
    public:
        CCliAgentMcu();
        ~CCliAgentMcu();

        void SetIO(CCliAgentMcuIoDevice *IoDevice);
        CCliAgentMcuIoDevice *GetIO();

    public:
        // did mac key oob这些信息在616上面是存储在mcu里面的,6441是存储在t41上，接口先放这里
        virtual int SetPSN(std::string strSN);
        virtual std::string GetPSN();
        virtual int SetSN(std::string strSN);
        virtual std::string GetSN();
        virtual int SetDID(std::string strDID);
        virtual std::string GetDID();
        virtual int SetPID(std::string strPID);
        virtual std::string GetPID();
        virtual int SetMAC(std::string strMAC);
        virtual std::string GetMAC();
        virtual int SetKey(std::string strKey);
        virtual std::string GetKey();
        virtual std::string GenOOB();
        virtual std::string GetOOB();
        virtual int SaveConfig();
        virtual int MCUUpgrade();

        virtual int BurnInStart();
        virtual int BurnInStop();
        virtual int WifiCmd(std::string strCmd);
        virtual std::string GetTxRxInfo();
        virtual std::string EnterSleepMode();
        virtual int FactoryReset();
        virtual int SysSHIPMode();
    public:
        virtual int RecvHandler(void *pvData);
        virtual std::string GetMcuVersion();
        virtual int GetBattery(int *piCapacity,int *piCurrent,int *piVoltage,int *piTemperature);
        virtual std::string GetBattery();
        virtual int LedSet(int iIndex,int iFlag);
        virtual std::string GetPirTriggerNum();
        virtual std::string StartPirTriggerNum();
        virtual std::string GetPirSignal();
        virtual std::string GetButtonLevel();
        virtual int SetBurnInMode(int iMode);
        virtual int SolarTestStart();
        virtual int SolarTestGetResult();
        virtual void StartLed();
        virtual void StopLed();
    private:
        CCliAgentMcuIoDevice *m_IoDevice;
    };

    class CCliAgentMcuFactory
    {
    public:
        static CCliAgentMcu *GetInstance(std::string type);

    private:
        CCliAgentMcuFactory();
        ~CCliAgentMcuFactory();

    private:
        static CCliAgentMcu *m_instance;
    };
}
#endif /* __CLI_AGENT_MCU__ */