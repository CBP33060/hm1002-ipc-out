#ifndef __CLI_AGENT_MCU_MT7628__
#define __CLI_AGENT_MCU_MT7628__

namespace maix {
    class CCliAgentMcuMT7628: public CCliAgentMcu
    {
    public:
        CCliAgentMcuMT7628();
        ~CCliAgentMcuMT7628();
		
	public:
        virtual int SetDID(std::string strDID);
        virtual std::string GetDID();
        virtual int SetMAC(std::string strMAC);
        virtual std::string GetMAC();
        virtual int SetKey(std::string strKey);
        virtual std::string GetKey();
        virtual std::string GenOOB();

        virtual int LedSet(int iIndex,int iFlag);

	private:

    };
}
#endif /* __CLI_AGENT_MCU_MT7628__ */