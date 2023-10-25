#include "cli_agent_host.hpp"
#include "cli_agent_host_t41.hpp"
#include "cli_agent_host_mt7628.hpp"

namespace maix {
    CCliAgentHost *CCliAgentHostFactory::m_instance = NULL;

    CCliAgentHost *CCliAgentHostFactory::GetInstance(std::string type)
    {
        if(m_instance != NULL)
        {
            return m_instance;
        }

        if(type.compare("t41") == 0)
        {
            m_instance = new CCliAgentHostT41();
        }
        else if(type.compare("mt7628") == 0)
        {
            m_instance = new CCliAgentHostMT7628(); 
        }

        return m_instance;
    }

    CCliAgentHost::CCliAgentHost()
    {
    }
    
    CCliAgentHost::~CCliAgentHost()
    {
    }

    /* * * * * * * * * API * * * * * * * * * */
    int CCliAgentHost::FactoryReset()
    {

    }

    int CCliAgentHost::BurnInStart()
    {
        return -1;
    }

    int CCliAgentHost::BurnInStop()
    {
        return -1;
    }

    int CCliAgentHost::AudioRecord(std::string strPath,int iFrameNum)
    {
        return -1;
    }

    int CCliAgentHost::AudioAplay(std::string strPath)
    {
        return -1;
    }
    
    int CCliAgentHost::LedSet(int iIndex,int iFlag)
    {
        return -1;
    }
    
    int CCliAgentHost::IrCutSet(int iFlag)
    {
        return -1;
    }    
    
    int CCliAgentHost::NightModeSet(int iFlag)
    {
        return -1;
    }
    
    std::string CCliAgentHost::GetAlsValue()
    {
        return "-1";
    }

    std::string CCliAgentHost::GetAlsRaw()
    {
        return "-1";
    }

    std::string CCliAgentHost::TestAls()
    {
        return "-1";
    }
    
    std::string CCliAgentHost::CalibrationAls()
    {
        return "-1";
    }
    
    std::string CCliAgentHost::GetVersion()
    {
        return "";
    }

    std::string CCliAgentHost::GetSdcardInfo()
    {
        return "";
    }
}