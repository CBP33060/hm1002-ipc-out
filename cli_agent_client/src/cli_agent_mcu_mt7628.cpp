#include "cli_agent_mcu.hpp"
#include "cli_agent_mcu_mt7628.hpp"

namespace maix {
    CCliAgentMcuMT7628::CCliAgentMcuMT7628()
    {

    }

    CCliAgentMcuMT7628::~CCliAgentMcuMT7628()
    {

    }

     /* * * * * * * * * * * * * * * * * * */
    int CCliAgentMcuMT7628::SetDID(std::string strDID)
    {
        return -1;
    }

    std::string CCliAgentMcuMT7628::GetDID()
    {
        return "";
    }

    int CCliAgentMcuMT7628::SetMAC(std::string strMAC)
    {
        return -1;
    }
    
    std::string CCliAgentMcuMT7628::GetMAC()
    {
        return "";
    }

    int CCliAgentMcuMT7628::SetKey(std::string strKey)
    {
        return -1;
    }

    std::string CCliAgentMcuMT7628::GetKey()
    {
        return "";
    }

    std::string CCliAgentMcuMT7628::GenOOB()
    {
        return "";
    }

    int CCliAgentMcuMT7628::LedSet(int iIndex,int iFlag)
    {
        return -1;
    }
}