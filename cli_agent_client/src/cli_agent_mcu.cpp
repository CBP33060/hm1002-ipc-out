#include "cli_agent_mcu.hpp"
#include "cli_agent_mcu_bl616.hpp"
#include "cli_agent_mcu_atbm6441.hpp"
#include "cli_agent_mcu_mt7628.hpp"

namespace maix {
    CCliAgentMcu *CCliAgentMcuFactory::m_instance = NULL;

    CCliAgentMcu *CCliAgentMcuFactory::GetInstance(std::string type)
    {
        if(m_instance != NULL)
        {
            return m_instance;
        }

        if(type.compare("bl616") == 0)
        {
            m_instance = new CCliAgentMcuBl616();
        }
        else if(type.compare("atbm6441") == 0)
        {
            m_instance = new CCliAgentMcuAtbm6441(); 
        }
        else if(type.compare("mt7628") == 0)
        {
            m_instance = new CCliAgentMcuMT7628(); 
        }

        return m_instance;
    }

    CCliAgentMcu::CCliAgentMcu()
    {
        m_IoDevice = NULL;
    }
    
    CCliAgentMcu::~CCliAgentMcu()
    {
        if(m_IoDevice != NULL)
        {
            delete m_IoDevice;
        }
    }

    void CCliAgentMcu::SetIO(CCliAgentMcuIoDevice *IoDevice)
    {
        m_IoDevice = IoDevice;
    }

    CCliAgentMcuIoDevice *CCliAgentMcu::GetIO()
    {
        return m_IoDevice;
    }

    /* * * * * * * * * * * * * * * * * * */

    int CCliAgentMcu::SetPSN(std::string strSN)
    {
        return -1;
    }

    std::string CCliAgentMcu::GetPSN()
    {
        return "";
    }

    int CCliAgentMcu::SetSN(std::string strSN)
    {
        return -1;
    }

    std::string CCliAgentMcu::GetSN()
    {
        return "";
    }

    int CCliAgentMcu::SetDID(std::string strDID)
    {
        return -1;
    }

    std::string CCliAgentMcu::GetDID()
    {
        return "";
    }

    int CCliAgentMcu::SetPID(std::string strPID)
    {
        return -1;
    }

    std::string CCliAgentMcu::GetPID()
    {
        return "";
    }

    int CCliAgentMcu::SetMAC(std::string strMAC)
    {
        return -1;
    }
    
    std::string CCliAgentMcu::GetMAC()
    {
        return "";
    }

    int CCliAgentMcu::SetKey(std::string strKey)
    {
        return -1;
    }

    std::string CCliAgentMcu::GetKey()
    {
        return "";
    }

    std::string CCliAgentMcu::GenOOB()
    {
        return "";
    }

    std::string CCliAgentMcu::GetOOB()
    {
        return "";
    }
 
    int CCliAgentMcu::WifiCmd(std::string strCmd)
    {
        return -1;
    }

    int CCliAgentMcu::BurnInStart()
    {
        return -1;
    }

    int CCliAgentMcu::BurnInStop()
    {
        return -1;
    }
    
    int CCliAgentMcu::SaveConfig()
    {
        return -1;
    }

    int CCliAgentMcu::MCUUpgrade()
    {
        return -1;
    }
    /* * * * * * * * * API * * * * * * * * * */
    int CCliAgentMcu::RecvHandler(void *pvData)
    {
        return -1;
    }
    
    std::string CCliAgentMcu::GetMcuVersion()
    {
        return "";
    }
    
    std::string CCliAgentMcu::GetTxRxInfo()
    {
        return "";
    }

    std::string CCliAgentMcu::EnterSleepMode()
    {
        return "";
    }

    int CCliAgentMcu::FactoryReset()
    {
        return 0;
    }

    int CCliAgentMcu::GetBattery(int *piCapacity,int *piCurrent,int *piVoltage,int *piTemperature)
    {
        return -1;
    }

    std::string CCliAgentMcu::GetBattery()
    {
        return "";
    }

    int CCliAgentMcu::LedSet(int iIndex,int iFlag)
    {
        return -1;
    }

    std::string  CCliAgentMcu::GetPirTriggerNum()
    {
        return "";
    }
    
    std::string  CCliAgentMcu::StartPirTriggerNum()
    {
        return "";
    }

    std::string CCliAgentMcu::GetPirSignal()
    {
        return "";
    }

    std::string  CCliAgentMcu::GetButtonLevel()
    {
        return "";
    }
    
    int CCliAgentMcu::SetBurnInMode(int iMode)
    {
        return -1;
    }

    int CCliAgentMcu::SysSHIPMode()
    {
        return -1;
    }

    int CCliAgentMcu::SolarTestStart()
    {
        return -1;
    }
    
    int CCliAgentMcu::SolarTestGetResult()
    {
        return -1;
    }

    void CCliAgentMcu::StartLed()
    {
        return ;
    }

    void CCliAgentMcu::StopLed()
    {
        return ;
    }

}