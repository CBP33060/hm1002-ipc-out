#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "cli/common.h"
#include "cli/console.h"
#include "cli_agent_client_config.hpp"
#include "cli/io_mux.h"
#include "cli/file_device.h"
#include "cli_agent_args.hpp"
#include "cli_agent_common.hpp"
#include "cli_agent_mcu.hpp"
#include "cli_agent_mcu_io_uart.hpp"
#include "cli_agent_host.hpp"
#include "cli_agent_host_t41.hpp"
#include "ncurses/curses.h"

#include "log_mx.h"
#include "common.h"
#include "fw_env_para.h"
#include "cli_agent_mcu_atbm6441.hpp"

static void FWParaConfigIncrease(const char *key)
{

    char *buf = maix::getFWParaConfig("user",key);
    int val = 0;

    if(buf != NULL)
    {
        val = atoi(buf);
    }

    val++;
    std::string strRet = std::to_string(val);
    maix::setFWParaConfig("user",key,strRet.c_str(),1);
}

static int GetFWParaConfigToInt(const char *key)
{
    char *buf = maix::getFWParaConfig("user",key);
    int val = 0;
    if(buf != NULL)
    {
        val = atoi(buf);
    }

    return val;
}

static void BurnInThreadLED()
{
    CREATE_HOST_INSTANCE

    std::string on = "wled on";
    std::string off = "wled off";

    while(1)
    {
        system(off.c_str());
        CliAgentHost->LedSet(1,1);
        sleep(120);
        CliAgentHost->LedSet(1,0);
        system(on.c_str());
        sleep(120);
    }

    CliAgentHost->LedSet(0,0);
    CliAgentHost->LedSet(1,0);
}

static void BurnInThreadLEDTest()
{
    CREATE_HOST_INSTANCE

    while(1)
    {
        CliAgentHost->LedSet(0,0);
        CliAgentHost->LedSet(1,1);
        sleep(20);
        CliAgentHost->LedSet(0,1);
        CliAgentHost->LedSet(1,0);
        sleep(20);
    }

    CliAgentHost->LedSet(0,0);
    CliAgentHost->LedSet(1,0);
}

static void BurnInThreadPilotLamp()
{
    CREATE_MCU_INSTANCE
    while(1)
    {
        CliAgentMcu->LedSet(0,1);
        sleep(3);
        CliAgentMcu->LedSet(2,1);
        sleep(3);
        CliAgentMcu->LedSet(1,1);
        sleep(3);
    }

    CliAgentMcu->LedSet(0,0);
}

static void BurnInThreadAudio()
{
    CREATE_HOST_INSTANCE
    while(1)
    {
        CliAgentHost->AudioAplay("/system/snd/pink_noise_test.wav");
        sleep(10);
    }
}

static void BurnInThreadAudioNoSleep()
{
    CREATE_HOST_INSTANCE
    while(1)
    {
        CliAgentHost->AudioAplay("/system/snd/speaker_to_mic_test.wav");
    }
}

static void BurnInThreadMicSpeaker()
{
    CREATE_HOST_INSTANCE
    while(1)
    {
        CliAgentHost->AudioRecord("/tmp/audio.wav",1500);
        CliAgentHost->AudioAplay("/tmp/audio.wav");
    }
}

static void BurnInThreadLight()
{
    CREATE_HOST_INSTANCE

    // FWParaConfigIncrease("als_oepn_times");
    std::string ret =CliAgentHost->GetAlsValue();
    int iPhotosensitiveValue = stoi(ret);
    if(iPhotosensitiveValue == -2)
    {
        FWParaConfigIncrease("als_oepn_failure_times");
        logPrint(MX_LOG_INFOR, "%s", "no device");
        maix::saveFWParaConfig();
        return;
    }

    while(1)
    {
        std::string ret =CliAgentHost->GetAlsRaw();
        int iPhotosensitiveValue = stoi(ret);
        if(iPhotosensitiveValue < 0)
        {
            FWParaConfigIncrease("als_read_failure_times");
            maix::saveFWParaConfig();
        }
        sleep(60);
    }
}

static void BurnInThreadIRCut()
{
    CREATE_HOST_INSTANCE
    while(1)
    {
        CliAgentHost->IrCutSet(1); 
        sleep(5);
        CliAgentHost->IrCutSet(0);
        sleep(5);
    }
}

static void BurnInThreadMic()
{
    CREATE_HOST_INSTANCE
    while(1)
    {
        CliAgentHost->AudioRecord("/tmp/audio.wav",75);
        sleep(10);
    }
}

static void BurnInSuccessLED()
{
    std::string on = "wled on 10000 1";
    std::string off = "wled off";

    while(1)
    {
        system(on.c_str());
        usleep(500000);
        system(off.c_str());
        usleep(500000);
        if(GetFWParaConfigToInt("burn_in_mode") == 0)
            break;
    }
    CREATE_MCU_INSTANCE
    CliAgentMcu->LedSet(1,1);
}

static void BurnInFailLED()
{
    std::string on = "irled on 10000 50";
    std::string off = "irled off";

    while(1)
    {
        system(on.c_str());
        usleep(500000);
        system(off.c_str());
        usleep(500000);
        if(GetFWParaConfigToInt("burn_in_mode") == 0)
            break;
    }
    CREATE_MCU_INSTANCE
    CliAgentMcu->LedSet(1,1);
}

static void BurnInPIR()
{
    CREATE_MCU_INSTANCE
    std::string on = "wled on 10000 25";
    std::string off = "wled off";
    
    while(1)
    {
        std::string str = CliAgentMcu->GetPirTriggerNum();
        usleep(300000);
        if(str != "")
        {
            system(on.c_str());
            usleep(100000);
            system(off.c_str());
            usleep(100000);
            system(on.c_str());
            usleep(100000);
            system(off.c_str());
            usleep(100000);
            system(on.c_str());
            usleep(100000);
            system(off.c_str());
            usleep(100000);
        }
    }
}

static int BurnInPowerOnCheck() 
{
    CREATE_MCU_INSTANCE

    int ret = 0;
    std::string strVal;

    if(!access("/tmp/_cli_agent_burn_in", F_OK))
    {
        return 0;
    }

    open("/tmp/_cli_agent_burn_in",O_RDWR | O_CREAT);

    int burn_in_runtime = GetFWParaConfigToInt("burn_in_runtime");
    int burn_in_total_runtime = GetFWParaConfigToInt("burn_in_total_runtime");
    int burn_in_time = GetFWParaConfigToInt("burn_in_time");
    int usb_pull_out = GetFWParaConfigToInt("usb_pull_out");
    int no_charge = GetFWParaConfigToInt("no_charge");

    burn_in_total_runtime += burn_in_runtime;
    
    if(usb_pull_out == 0 && no_charge == 0 && burn_in_runtime != 0 && burn_in_runtime != 30)
    {
        // std::ostringstream oss;
        // oss << burn_in_total_runtime;
        // std::string str = oss.str();
        // const char* RestarTime  = str.c_str();
        //FWParaConfigIncrease("burn_in_abnormal_reboot_times");
        // maix::setFWParaConfig("user",RestarTime,"RestarTime",1);
    }

    strVal = std::to_string(burn_in_total_runtime);
    maix::setFWParaConfig("user","burn_in_total_runtime",strVal.c_str(),1);
    maix::setFWParaConfig("user","burn_in_runtime","0",1);

    if(burn_in_total_runtime >= burn_in_time 
        || usb_pull_out > 0
        || no_charge > 0)
    {
        maix::setFWParaConfig("user","burn_in_flag","2",1);
        struct s_burn_in_info info;
        GetBurnInInfo(&info);

        if(info.burn_in_result != 0)
        {
            maix::setFWParaConfig("user","burn_in_mode","2",1);
            // CliAgentMcu->LedSet(0,1);
            // static std::thread threadFail(BurnInFailLED);
            ret = 1;
        }
        else
        {
            maix::setFWParaConfig("user","burn_in_mode","3",1);
            ret = 2;
            // maix::setFWParaConfig("user","burn_in_success","1",1);
            // static std::thread threadSuccess(BurnInSuccessLED);
        }
    }

    maix::saveFWParaConfig();
    return ret;
}

static void threadRestart()
{
    CREATE_MCU_INSTANCE
    
    int burn_in_runtime = GetFWParaConfigToInt("burn_in_runtime");

    for (int i = burn_in_runtime; i < 30; i++)
    {
        FWParaConfigIncrease("burn_in_runtime");
        maix::saveFWParaConfig();
        sleep(60);
    }
    printf("=====================================================rebooting================================================\n");
    system("touch /tmp/tag_env_info_lock");
    system("fw_setenv lock"); //添加env lock 防止进低功耗时env分区正在写被损坏
    sleep(3);
    std::string strCmd = "AT+REBOOT";
    CliAgentMcu->WifiCmd(strCmd);
    // system("reboot");
}

void GetBurnInInfo(struct s_burn_in_info *info)
{
    info->camera_open_times = 0;
    info->burn_in_total_runtime = 0;
    info->burn_in_runtime = 0;
    info->burn_in_abnormal_reboot_times = 0;
    info->als_oepn_failure_times = 0;
    info->als_read_failure_times = 0;
    info->camera_open_failure_times = 0;
    info->camera_read_failure_times = 0;
    info->usb_pull_out = 0;
    info->no_charge = 0;
    info->burn_in_result = 0;

    char *buf = maix::getFWParaConfig("user","camera_open_times");
    if(buf != NULL)
    {
        info->camera_open_times = atoi(buf);
    }

    buf = maix::getFWParaConfig("user","burn_in_total_runtime");
    if(buf != NULL)
    {
        info->burn_in_total_runtime = atoi(buf);
    }

    buf = maix::getFWParaConfig("user","burn_in_runtime");
    if(buf != NULL)
    {
        info->burn_in_runtime = atoi(buf);
    }

    buf = maix::getFWParaConfig("user","burn_in_abnormal_reboot_times");
    if(buf != NULL)
    {
        info->burn_in_abnormal_reboot_times = atoi(buf);
        if( atoi(buf) > 0 )
        {
            info->burn_in_result = 1;
        }
    }

    buf = maix::getFWParaConfig("user","als_oepn_failure_times");
    if(buf != NULL)
    {
        info->als_oepn_failure_times = atoi(buf);
        if( atoi(buf) > 61 )
        {
            info->burn_in_result = 1;
        }
    }

    buf = maix::getFWParaConfig("user","als_read_failure_times");
    if(buf != NULL)
    {
        info->als_read_failure_times = atoi(buf);
        if( atoi(buf) > 0 )
        {
            info->burn_in_result = 1;
        }
    }

    buf = maix::getFWParaConfig("user","camera_open_failure_times");
    if(buf != NULL)
    {
        info->camera_open_failure_times = atoi(buf);
        if( atoi(buf) > 3 )
        {
            info->burn_in_result = 1;
        }
    }

    buf = maix::getFWParaConfig("user","camera_read_failure_times");
    if(buf != NULL)
    {
        info->camera_read_failure_times = atoi(buf);
        if( atoi(buf) > 0 )
        {
            info->burn_in_result = 1;
        }
    }

    buf = maix::getFWParaConfig("user","usb_pull_out");
    if(buf != NULL)
    {
        info->usb_pull_out = atoi(buf);
        if( atoi(buf) > 0 )
        {
            info->burn_in_result = 1;
        }
    }

    buf = maix::getFWParaConfig("user","no_charge");
    if(buf != NULL)
    {
        info->no_charge = atoi(buf);
        if( atoi(buf) > 0 )
        {
            info->burn_in_result = 1;
        }
    }
}

void UsbCheck(int BurnInMode)
{
    int state = 0;
    int times = 0;
    int capacityOld = 0;
    int charge_count = 0;
    int usb_pull_count = 0;
    int result_thread_start = 0;
    int usbFlag = 0;
    int capacity = 0;
    int voltage = 0;
    int current = 0;
    int NTC = 0;
    int cnt = 0;
    
    CREATE_MCU_INSTANCE
    while(1)
    {
        std::string strMsg = CliAgentMcu->GetBattery();
        if(strMsg.length() == 0)
            continue;
        sscanf(strMsg.c_str(),"+FACT:usb:%d,soc:%d,vol:%d,cur:%d,NTC:%d,count:%d" ,&usbFlag,&capacity,&voltage,&current,&NTC,&cnt);
        // printf("usb:%d,state:%d\n",usbFlag,state);
        // 老化成功之后需要检测电量为65才闪灯
        if(result_thread_start == 0 && BurnInMode == 3) {
            if(capacity < 65) { // 老化成功之后但电没有充到65，三色灯循环亮
                static int led_flag = 0;
                CliAgentMcu->LedSet(led_flag++ % 3,1);
            } else {
                CliAgentMcu->LedSet(0,0);
                static std::thread threadSuccess(BurnInSuccessLED);
                result_thread_start = 1;
            }
        } else if(result_thread_start == 0 && BurnInMode == 2) {
            CliAgentMcu->LedSet(0,0);
            static std::thread threadFail(BurnInFailLED);
            result_thread_start = 1;
        }

        if(BurnInMode == 1 && ++times % 60 == 0) {
            if(capacity < 60 && capacity < capacityOld) {
                capacityOld = capacity;
                charge_count++;
                if(charge_count > 5) {
                    // 設置usb 沒有充電
                    maix::setFWParaConfig("user","no_charge","1",1);
                    exit(1);
                }
            } else {
                charge_count = 0;
                capacityOld = capacity;
            }
        }

        switch(state)
        {
        case 0://第一次插入USB
            if( usbFlag == 1 ) { 
                state = 1;
            } //第一次上电没有插入USB
            else if( usbFlag == 0 ) {
                //老化模式不应该出现这种情况
                // if(BurnInMode == 1) {
                //     // 設置usb拔出老化失敗標志位
                //     maix::setFWParaConfig("user","usb_pull_out","1",1);
                //     exit(1);
                // }
                // printf("AAAAAAAAAAAAAAAA\n");
                state = 2;
            }
            break;
        case 1://拔掉usb
            if( usbFlag == 0 ) {
                //正在老化
                if(BurnInMode == 1) {
                    usb_pull_count++;
                    if(usb_pull_count > 30) {
                        // 設置usb拔出老化失敗標志位
                        maix::setFWParaConfig("user","usb_pull_out","1",1);
                        system("wled off");
                        system("irled off");
                        exit(1);
                    }
                } else {
                    state = 2;
                }
            } else {
                usb_pull_count = 0;
            }
            break;
        case 2://再次接入USB
            if( usbFlag == 1 ) { 
                exit(1);
            }
            break;
        }
        sleep(1);
    }
}

void BurnInHandle()
{
    CREATE_MCU_INSTANCE
    system("fw_setenv unlock");
    int BurnInMode = GetFWParaConfigToInt("burn_in_mode");// 0:没有老化 1：正在老化 2：老化失败 3：老化成功
    CliAgentMcu->SetBurnInMode(BurnInMode);
    if(BurnInMode == 1 && BurnInPowerOnCheck() == 0)
    {
        // 老化模式等待插入USB在开始执行
        std::string strMsg = CliAgentMcu->GetBattery();
        int usbFlag = 0;
        int capacity = 0;
        int voltage = 0;
        int current = 0;
        int NTC = 0;
        int cnt = 0;
        sscanf(strMsg.c_str(),"+FACT:usb:%d,soc:%d,vol:%d,cur:%d,NTC:%d,count:%d" ,&usbFlag,&capacity,&voltage,&current,&NTC,&cnt);
        while(usbFlag == 0)
        {
            sleep(5);
            strMsg = CliAgentMcu->GetBattery();
            sscanf(strMsg.c_str(),"+FACT:usb:%d,soc:%d,vol:%d,cur:%d,NTC:%d,count:%d" ,&usbFlag,&capacity,&voltage,&current,&NTC,&cnt);
        }

        printf("burn in mode\n");
        static std::thread threadLED(BurnInThreadLED);
        static std::thread threadPilotLamp(BurnInThreadPilotLamp);
        static std::thread threadAudio(BurnInThreadAudio);
        static std::thread threadLight(BurnInThreadLight);
        static std::thread threadIRCut(BurnInThreadIRCut);
        static std::thread threadMic(BurnInThreadMic);
        static std::thread threadReboot(threadRestart);
    }
    else if(BurnInMode == 0)
    {
        CliAgentMcu->LedSet(1,1);
        if(GetFWParaConfigToInt("burn_in_mic_speak"))
        {
            static std::thread threadMic(BurnInThreadMicSpeaker);
        }
        else
        {
            if(GetFWParaConfigToInt("burn_in_mic"))
            {
                static std::thread threadMic(BurnInThreadMic);
            }

            if(GetFWParaConfigToInt("burn_in_speak"))
            {
                static std::thread threadAudio(BurnInThreadAudio);
            }

            if(GetFWParaConfigToInt("burn_in_mic_speak_no_sleep"))
            {
                static std::thread threadMic(BurnInThreadAudioNoSleep);
            }
        }

        if(GetFWParaConfigToInt("burn_in_mic_ircut"))
        {
            static std::thread threadIRCut(BurnInThreadIRCut);
        }

        if(GetFWParaConfigToInt("burn_in_mic_led"))
        {
            static std::thread threadLED(BurnInThreadLEDTest);
            static std::thread threadRGB(BurnInThreadPilotLamp);
        }

        if(GetFWParaConfigToInt("burn_in_pir"))
        {
            static std::thread threadPIR(BurnInPIR);
        }
    }

    BurnInMode = GetFWParaConfigToInt("burn_in_mode");
    static std::thread threadUsbCheck(UsbCheck,BurnInMode);
}