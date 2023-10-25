#include <string.h>
#include <string.h>
#include <sstream>

#include "cli_agent_api.hpp"
#include "cli_agent_mcu.hpp"
#include "cli_agent_common.hpp"
#include "cli_agent_host.hpp"
#include "log_mx.h"
#include "fw_env_para.h"
#include "common.h"
#include <iostream>
#include "cli_agent_mcu_atbm6441.hpp"
#include "fw_env_para.h"

extern "C" {
	#include "efuse.h"
}

#define _OK "<OK>\n"
#define _ERR "<ERROR>\n"
#define _END "\n"

CREATE_MCU_INSTANCE_LED
static void PrintResult(const cli::OutputDevice& CLI_Out,int code,std::string strResult)
{
    // usleep(500 * 1000);
    CLI_Out << strResult << _END;
    usleep(500 * 1000);
    if(code == 0)
    {
        CLI_Out << _OK;
    }
    else
    {
        CLI_Out << _ERR;
    }

}

void QuitTest(const cli::OutputDevice& CLI_Out)
{

}

void FacReset(const cli::OutputDevice& CLI_Out)
{
    std::string cmd = "tag_env_info --set HW 70mai_factory_mode 0";
    system(cmd.c_str());

    CREATE_MCU_INSTANCE
    CREATE_HOST_INSTANCE
    CliAgentMcu->FactoryReset();
    CliAgentHost->FactoryReset();
    CliAgentMcu->SysSHIPMode();

    PrintResult(CLI_Out,0,"");
}

void BurnInStart(const cli::OutputDevice& CLI_Out,const char* time)
{
    CREATE_MCU_INSTANCE

    maix::setFWParaConfig("user","camera_open_times","0",1);
    maix::setFWParaConfig("user","burn_in_total_runtime","0",1);
    maix::setFWParaConfig("user","burn_in_runtime","0",1);
    maix::setFWParaConfig("user","burn_in_abnormal_reboot_times","0",1);
    maix::setFWParaConfig("user","als_oepn_failure_times","0",1);
    maix::setFWParaConfig("user","als_read_failure_times","0",1);
    maix::setFWParaConfig("user","camera_open_failure_times","0",1);
    maix::setFWParaConfig("user","camera_read_failure_times","0",1);
    maix::setFWParaConfig("user","usb_pull_out","0",1);
    maix::setFWParaConfig("user","no_charge","0",1);
    
    maix::setFWParaConfig("user","burn_in_mode","1",1); // 0:没有老化 1：正在老化 2：老化失败 3：老化成功，读取结果后设置为0，代表正式退出老化，不在闪灯
    maix::setFWParaConfig("user","burn_in_time",time,1);
    maix::setFWParaConfig("user","burn_in_flag","1",1); // 0:没有老化 1：正在老化 2：老化完成 标志位保留，除非再次开启老化
    maix::saveFWParaConfig();

    CliAgentMcu->SysSHIPMode();

    PrintResult(CLI_Out,0,"");
}

void BurnInStop(const cli::OutputDevice& CLI_Out)
{
    maix::setFWParaConfig("user","burn_in_mode","0",1);
    maix::setFWParaConfig("user","burn_in_flag","0",1);
    maix::saveFWParaConfig();

    PrintResult(CLI_Out,0,"");
}

void GetBurnInResult(const cli::OutputDevice& CLI_Out)
{
    char *buf = maix::getFWParaConfig("user","burn_in_flag");
    int val = 0;
    if(buf != NULL)
    {
        val = atoi(buf);
    }

    if(val == 0)
    {
        CLI_Out << "No burn-in";
        CLI_Out << _END;
        CLI_Out << _ERR;
        return ;
    }
    else if(val == 1)
    {
        CLI_Out << "burn-in doing";
        CLI_Out << _END;
        CLI_Out << _ERR;
        return ;
    }

    struct s_burn_in_info info;
    GetBurnInInfo(&info);
    if(info.burn_in_result != 0)
    {
        CLI_Out << "burn_in_abnormal_reboot_times:" << info.burn_in_abnormal_reboot_times;
        CLI_Out << " als_oepn_failure_times:" << info.als_oepn_failure_times;
        CLI_Out << " als_read_failure_times:" << info.als_read_failure_times;
        CLI_Out << " camera_open_failure_times:" << info.camera_open_failure_times;
        CLI_Out << " camera_read_failure_times:" << info.camera_read_failure_times;
        CLI_Out << " usb_pull_out:" << info.usb_pull_out;
        CLI_Out << " no_charge:" << info.no_charge;
        CLI_Out << _END;
        CLI_Out << _ERR;
    }
    else 
    {
        PrintResult(CLI_Out,0,"");  
    }
    maix::setFWParaConfig("user","burn_in_mode","0",1);
    maix::saveFWParaConfig();
}

void WIFICMD(const cli::OutputDevice& CLI_Out,const char* chAT)
{
    CREATE_MCU_INSTANCE
    
    int ret = CliAgentMcu->WifiCmd(chAT);

    PrintResult(CLI_Out,ret,"");
}

void GetVersion(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    CREATE_MCU_INSTANCE
    std::string strHostVersion = CliAgentHost->GetVersion();
    std::string strMcuVersion = CliAgentMcu->GetMcuVersion();
    PrintResult(CLI_Out,0,strHostVersion + " " + strMcuVersion);
}

void GenOOB(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE

    CliAgentMcu->GenOOB();
    std::string strOob = CliAgentMcu->GenOOB();
    
    PrintResult(CLI_Out,0,strOob);
}

void SetPSN(const cli::OutputDevice& CLI_Out,const char* psn)
{
    CREATE_MCU_INSTANCE
    int ret = CliAgentMcu->SetPSN(psn);
    maix::saveFWParaConfig();

    PrintResult(CLI_Out,ret,"");
}

void GetPSN(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string psn = CliAgentMcu->GetPSN();

    PrintResult(CLI_Out,0,psn);
}

void SetSN(const cli::OutputDevice& CLI_Out,const char* sn)
{
    CREATE_MCU_INSTANCE
    int ret = CliAgentMcu->SetSN(sn);
    maix::saveFWParaConfig();

    PrintResult(CLI_Out,ret,"");
}

void GetSN(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string sn = CliAgentMcu->GetSN();

    PrintResult(CLI_Out,0,sn);
}

void SetKey(const cli::OutputDevice& CLI_Out,const char* key)
{
    CREATE_MCU_INSTANCE
    int ret = 0;
    std::string str_key = CliAgentMcu->GetKey();
    if(str_key.length() == 0 )  
    {
        ret = CliAgentMcu->SetKey(key);
        maix::saveFWParaConfig();        
    } 
    else if(strcmp(key,str_key.c_str()) != 0)
    {
        ret = 1;
        PrintResult(CLI_Out,ret,"Different Key");
        return;
    }
    PrintResult(CLI_Out,ret,"");
}

void GetKey(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string key = CliAgentMcu->GetKey();

    PrintResult(CLI_Out,0,key);
}

void SetDID(const cli::OutputDevice& CLI_Out,const char* did)
{
    CREATE_MCU_INSTANCE
    std::string str_key = CliAgentMcu->GetDID();
    int ret = 0;
    if(str_key.length() == 0 )
    {
        ret = CliAgentMcu->SetDID(did);
        maix::saveFWParaConfig();
    } 
    else if(strcmp(did,str_key.c_str()) != 0)
    {
        ret = 1;
        PrintResult(CLI_Out,ret,"Different DID");
        return;
    }
    PrintResult(CLI_Out,ret,"");
}

void GetDID(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE

    std::string did = CliAgentMcu->GetDID();

    PrintResult(CLI_Out,0,did);
}

void SetPID(const cli::OutputDevice& CLI_Out,const char* pid)
{
    CREATE_MCU_INSTANCE

    int ret = CliAgentMcu->SetPID(pid);
    
    maix::saveFWParaConfig();

    PrintResult(CLI_Out,ret,"");
}

void GetPID(const cli::OutputDevice& CLI_Out)
{
   CREATE_MCU_INSTANCE

    std::string pid = CliAgentMcu->GetPID();

    PrintResult(CLI_Out,0,pid);
}

void SetMAC(const cli::OutputDevice& CLI_Out,const char* mac)
{
    CREATE_MCU_INSTANCE

    int ret = CliAgentMcu->SetMAC(mac);

    PrintResult(CLI_Out,ret,"");
}

void GetMAC(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE

    std::string mac = CliAgentMcu->GetMAC();

    PrintResult(CLI_Out,0,mac);
}

void SaveConfig(const cli::OutputDevice& CLI_Out)
{
   CREATE_MCU_INSTANCE
    CliAgentMcu->SaveConfig();

    PrintResult(CLI_Out,0,"");
}

std::string RemoveColons(const std::string& mac_address) {
    std::ostringstream result;
    for (char c : mac_address) {
        if (c != ':') {
            result << c;
        }
    }
    return result.str();
}

void GetQRCode(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE

    char QRCode[258];
    memset(QRCode, 0, sizeof(QRCode));
    std::string strPID = CliAgentMcu->GetPID();
    std::string strDID = CliAgentMcu->GetDID();
    std::string strMAC = CliAgentMcu->GetMAC();  
    std::string strNewMAC =  RemoveColons(strMAC);
    std::string strOOB = CliAgentMcu->GetOOB();  
    sprintf(QRCode,"https://home.mi.com/do/home.html?f=xz&p=%s&d=%s&m=%s&O=%s", strPID.c_str(),strDID.c_str(), strNewMAC.c_str(), strOOB.c_str());
    PrintResult(CLI_Out,0,QRCode);
}

void MICTest(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    int ret = CliAgentHost->AudioRecord("/tmp/audio.wav",75);

    PrintResult(CLI_Out,ret,"");
}

void MICSpkTest(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    int ret1 = CliAgentHost->AudioRecord("/tmp/audio.wav",75);
    int ret2 = CliAgentHost->AudioAplay("/tmp/audio.wav");
    
    PrintResult(CLI_Out, ret1 || ret2,"");
}

void MICFileUpload(const cli::OutputDevice& CLI_Out)
{
    int fd = open("/tmp/audio.wav",O_RDONLY);
    if(fd <= 0)
    {
        CLI_Out << _ERR;
        return;
    }

    int size = lseek(fd,0,SEEK_END);

    CLI_Out << _OK;
    CLI_Out << size << cli::endl;

    lseek(fd,0,SEEK_SET);

    char buf[512];
    int ret = 0;

    do
    {
        ret = read(fd,buf,sizeof(buf));
        CLI_Out.PutBuffer(buf,sizeof(buf));
    }
    while(ret > 0);
}

void RGBLEDSet(const cli::OutputDevice& CLI_Out,int index,int flag)
{
    CREATE_MCU_INSTANCE

    int ret = CliAgentMcu->LedSet(index,flag);
    PrintResult(CLI_Out,ret,"");
}

void RGBLEDStart(const cli::OutputDevice& CLI_Out)
{
    CliAgentMcuLed->StartLed();

    PrintResult(CLI_Out,0,"");
}

void RGBLEDStop(const cli::OutputDevice& CLI_Out)
{
    CliAgentMcuLed->StopLed();
    
    PrintResult(CLI_Out,0,"");
}

void IrCutSet(const cli::OutputDevice& CLI_Out,int flag)
{
    CREATE_HOST_INSTANCE

    int ret = CliAgentHost->IrCutSet(flag); 

    PrintResult(CLI_Out,ret,"");
}

void NightModeSet(const cli::OutputDevice& CLI_Out,int flag)
{
    CREATE_HOST_INSTANCE

    int ret = CliAgentHost->NightModeSet(flag); 

    PrintResult(CLI_Out,ret,"");
}

void SPKTest(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE

    int ret = CliAgentHost->AudioAplay("/system/snd/100-20kHz_0dB.wav");
    
    PrintResult(CLI_Out, ret,"");
}

static void AudioAplay()
{
    CREATE_HOST_INSTANCE
    CliAgentHost->AudioAplay("/system/snd/speaker_to_mic_test.wav");
}

static void AudioRecord()
{
    CREATE_HOST_INSTANCE
    CliAgentHost->AudioRecord("/tmp/audio.wav",75);
}

void MICSpkAec(const cli::OutputDevice& CLI_Out)
{
    std::thread SPKStartAplay(AudioAplay);
    std::thread SPKStartRecord(AudioRecord);
    
    SPKStartAplay.join();
    SPKStartRecord.join();
   
    PrintResult(CLI_Out, 0,"");
}

void CalibrationAls(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    std::string ret = CliAgentHost->CalibrationAls();
    int val = atoi(ret.c_str());
    
    if(val >= 0)
    {
        PrintResult(CLI_Out, 0,ret);
    }
    else
    {
        PrintResult(CLI_Out, 1,"");
    }
}

void GetAlsInterrupt(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    std::string ret = CliAgentHost->TestAls();
    int val = atoi(ret.c_str());
    
    if(val >= 0)
    {
        PrintResult(CLI_Out, 0,ret);
    }
    else
    {
        PrintResult(CLI_Out, 1,"");
    }
}

void GetAlsValue(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    std::string ret = CliAgentHost->GetAlsValue();
    int val = atoi(ret.c_str());
    
    if(val >= 0)
    {
        PrintResult(CLI_Out, 0,ret);
    }
    else
    {
        PrintResult(CLI_Out, 1,"");
    }
}

void GetAlsRaw(const cli::OutputDevice& CLI_Out)
{
    CREATE_HOST_INSTANCE
    std::string ret = CliAgentHost->GetAlsRaw();
    int val = atoi(ret.c_str());
    
    if(val >= 0)
    {
        PrintResult(CLI_Out, 0,ret);
    }
    else
    {
        PrintResult(CLI_Out, 1,"");
    }
}

void LightSensorStop(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "LightSensorStop" << cli::endl;

    maix::logPrint(maix::MX_LOG_ERROR, "LightSensorStop tid = %u",pthread_self());
}

void GravitySensorStart(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "GravitySensorStart" << cli::endl;
}

void Wled(const cli::OutputDevice& CLI_Out,int flag)
{
    CREATE_HOST_INSTANCE
    CliAgentHost->LedSet(0,flag);
    CLI_Out << _OK << cli::endl;
}

void Irled(const cli::OutputDevice& CLI_Out,int flag)
{
    CREATE_HOST_INSTANCE
    CliAgentHost->LedSet(1,flag);
    CLI_Out << _OK << cli::endl;
}

void GetPirTriggerNum(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string ret = CliAgentMcu->GetPirTriggerNum();
    PrintResult(CLI_Out,0,ret);
}

void GetPirSignal(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string ret = CliAgentMcu->GetPirSignal();
    PrintResult(CLI_Out,0,ret);
}

void StartPirTriggerNum(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string ret = CliAgentMcu->StartPirTriggerNum();
    PrintResult(CLI_Out,0,ret);
}

void GetBattery(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    int Capacity = 0,Current = 0,Voltage = 0,Temperature = 0;
    CliAgentMcu->GetBattery(&Capacity,&Current,&Voltage,&Temperature);
    CLI_Out << _OK << "Capacity:" << Capacity << " Current:" <<  Current << " Voltage:" <<  Voltage <<" Temperature:" <<  Temperature << cli::endl;
}

void GetButtonLevel(const cli::OutputDevice& CLI_Out)
{   
    CREATE_MCU_INSTANCE
 
    for(int i = 0; i < 35; i++)
    {
	    std::string strRet = CliAgentMcu->GetButtonLevel();

        int ret = strRet.find("sta:1");
        if(ret > 0)
        {
            CLI_Out << "P\r\n<ok>\n";
	        return ;
	    }
    }

    PrintResult(CLI_Out,1,"");

}

void SaveInfo(const cli::OutputDevice& CLI_Out,const char* mkey,const char* mvalue)
{
    maix::setFWParaConfig("factory",mkey,mvalue,1);
    maix::saveFWParaConfig();

    PrintResult(CLI_Out,0,"");
}

void GetInfo(const cli::OutputDevice& CLI_Out,const char* mkey)
{
    // 存储每条测试指令的结果到一个文件里面，以key=value的形式存储，直接将文件返回
    std::string ret = maix::getFWParaConfig("factory",mkey);
    PrintResult(CLI_Out,0,ret);
}

void MCUUpgrade(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    int ret = CliAgentMcu->MCUUpgrade();
    std::string msg = "";
    if(ret == 1)
    {
        msg = "upgradeing";
        ret = 0;
    }
    else if(ret == 2)
    {
        msg = "done";
        ret = 0;
    }

    PrintResult(CLI_Out,ret,msg);
}

void SysSHIPMode(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    int ret = CliAgentMcu->SysSHIPMode();

    PrintResult(CLI_Out,ret,"");
}

void GetRxInfo(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE

    PrintResult(CLI_Out,0,CliAgentMcu->GetTxRxInfo());
}

void EnterSleepMode(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE

    PrintResult(CLI_Out,0,CliAgentMcu->EnterSleepMode());
}

void BurnOtp(const cli::OutputDevice& CLI_Out)
{
    int ret = writeRsaMode(65537,"/system/rsa_mod_n_sha256.bin");
    PrintResult(CLI_Out,ret,"");
}

void SolarTestStart(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    int ret = CliAgentMcu->SolarTestStart();
    PrintResult(CLI_Out,ret,"");
}

void SolarTestGetResult(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    int ret = CliAgentMcu->SolarTestGetResult();
    PrintResult(CLI_Out,0,std::to_string(ret));
}

void GetMcuVersion(const cli::OutputDevice& CLI_Out)
{
    CREATE_MCU_INSTANCE
    std::string strVersion = CliAgentMcu->GetMcuVersion();
    PrintResult(CLI_Out,0,strVersion);
}

void MCUTest(const cli::OutputDevice& CLI_Out, int iCmdCode)
{
    CREATE_MCU_INSTANCE
    
    if(iCmdCode == 1)
    {
        maix::CCliAgentMcuIoDevice *CliAgentMcuIo = CliAgentMcu->GetIO();
        char buf[128] = {1,2,3,4,5,6,7,8,9,10};
        int ret = CliAgentMcuIo->SendBuf((unsigned char *)buf,10);

        cli::OutputDevice::GetStdErr() << "send ret:" << ret << cli::endl;

        memset(buf,0,sizeof(buf));
        ret = CliAgentMcuIo->RecvBuf((unsigned char *)buf,sizeof(buf),3000);

        cli::OutputDevice::GetStdErr() << "recv ret:" << ret << cli::endl;

        CLI_Out << _OK << "Recv:" << buf << cli::endl;
    }
    else if(iCmdCode == 2)
    {
        std::string Version = CliAgentMcu->GetMcuVersion();
        CLI_Out << _OK << "version:" << Version << cli::endl;
    }
    else if(iCmdCode == 3)
    {
        int Capacity = 0,Current = 0,Voltage = 0,Temperature = 0;
        CliAgentMcu->GetBattery(&Capacity,&Current,&Voltage,&Temperature);
        CLI_Out << _OK << "Capacity:" << Capacity << " Current:" <<  Current << " Voltage:" <<  Voltage <<" Temperature:" <<  Temperature << cli::endl;
        PrintResult(CLI_Out,0,CliAgentMcu->GetBattery());
    }
    else if(iCmdCode == 4)
    {
        PrintResult(CLI_Out,0,CliAgentMcu->GetPirTriggerNum());
    }
    else if(iCmdCode == 5)
    {
        PrintResult(CLI_Out,0,CliAgentMcu->GetButtonLevel());
    }
    else if(iCmdCode == 6)
    {
        PrintResult(CLI_Out,0,CliAgentMcu->GetMcuVersion());
    }
    else
    {
        CLI_Out << _ERR << cli::endl;
    }
}
