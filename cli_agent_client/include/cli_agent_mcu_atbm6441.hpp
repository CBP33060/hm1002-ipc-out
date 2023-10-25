#ifndef __CLI_AGENT_MCU_ATBM6441__
#define __CLI_AGENT_MCU_ATBM6441__

#include <mutex>
#include <thread>
#include <atomic>

namespace maix {
    class CCliAgentMcuAtbm6441: public CCliAgentMcu
    {
    public:
        CCliAgentMcuAtbm6441();
        ~CCliAgentMcuAtbm6441();

	public:
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
		int SendToAtbmd(std::string strCommand);
		int SendFactoryCmd(std::string strCommand);
		virtual void StartLed();
        virtual void StopLed();
		void ThreadLed();
    private:
		int GetMcuDeviceState();
		int GetMcuDeviceInfo();
		int RecvMcuMsg(int iTimeout);
		int RecvMcuMsg(int iTimeout,std::string debug);
		int RecvSystemPackte(int iId,void *pvData);
		int RecvIpcPackte(int iId,void *pvData);
		int RecvFactoryPackte(int iId,void *pvData);
		int SendBuf(unsigned char *pBuf, int iLen);
		int RecvBuf(unsigned char *pBuf, int iLen, int iTimeout);
	public:
		int m_iUpgradeFlag;
	private:
		unsigned char *m_pRecvBuf;
		int m_iRecvBufLen;

		std::mutex m_mutexMcuUart;
		std::string m_strFactoryResponse;
		std::string m_strMcuVersion;
		int m_iCapacity;
		int m_iCurrent;
		int m_iVoltage;
		int m_iTemperature;
		int m_iOOBFlag;
		std::thread m_ledthread;
		std::atomic<bool> m_bshouldExit;
		std::atomic<bool> m_bRunning;
    };
}
#endif /* __CLI_AGENT_MCU_ATBM6441__ */