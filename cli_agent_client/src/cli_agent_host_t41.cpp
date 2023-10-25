#include "cli/pch.h"
#include "cli/common.h"
#include "cli_agent_common.hpp"
#include "log_mx.h"
#include "cli_agent_host.hpp"
#include "cli_agent_host_t41.hpp"
#include "common.h"
#include "ini_config.h"
#include "fw_env_para.h"
#include <sys/time.h>

namespace maix {

    CCliAgentHostT41::CCliAgentHostT41()
    {
        m_cAiInterface = new CZeratulAudioInterface("ai");
        m_cAoInterface = new CZeratulSpeakerInterface("ao");

        if(m_cAiInterface == NULL)
        {
            cli::OutputDevice::GetStdErr() << "m_cAiInterface:new failure" << cli::endl;
        }
        else
        {
            m_cAiInterface->init();
        }

        if(m_cAoInterface == NULL)
        {
            cli::OutputDevice::GetStdErr() << "m_cAoInterface:new failure" << cli::endl;
        }
        else
        {
            m_cAoInterface->init();
        }
    }

    CCliAgentHostT41::~CCliAgentHostT41()
    {
        if(m_cAiInterface != NULL)
        {
            m_cAiInterface->unInit();
            delete m_cAiInterface;
        }
        
        if(m_cAoInterface != NULL)
        {
            m_cAoInterface->unInit();
            delete m_cAoInterface;
        }
    }

    int CCliAgentHostT41::AudioRecord(std::string strPath,int iFrameNum)
    {

        int fd = open(strPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
        if(fd <= 0)
        {
            cli::OutputDevice::GetStdErr() << "AudioRecord:open failure" << cli::endl;
            return -2;
        }

        // if(m_cAiInterface == NULL)
        // {
        //     cli::OutputDevice::GetStdErr() << "m_cAiInterface:new failure" << cli::endl;
        //     close(fd);
        //     return -3;
        // }

        // m_cAiInterface->init();

        int ret_size = 0,data_len = 0;
        unsigned char *tmp;
        unsigned char _tmp_buf[WAV_HEAD_LEN] = {0};
        
        write(fd,_tmp_buf,WAV_HEAD_LEN);
        
        struct timeval prevEnd, currEnd;

        while(1)
        {
            int i = 0;
        	gettimeofday(&prevEnd, NULL); 
            tmp = m_cAiInterface->readFrame(0,&ret_size);
            gettimeofday(&currEnd, NULL);
		    long int timeDiff = (currEnd.tv_sec - prevEnd.tv_sec) * 1000 + (currEnd.tv_usec - prevEnd.tv_usec) / 1000;
            // printf("timeDiff:%ld==========\n", timeDiff);
            if(timeDiff > 0)
            {
                break;
            }
        }
		

        for(int i = 0;i < iFrameNum;i++)
        {
		    gettimeofday(&prevEnd, NULL); 
            tmp = m_cAiInterface->readFrame(0,&ret_size);
            gettimeofday(&currEnd, NULL);  // 获取当前发送结束时间
		    // 计算相邻两次发送时间差（以毫秒为单位）
		    long int timeDiff = (currEnd.tv_sec - prevEnd.tv_sec) * 1000 + (currEnd.tv_usec - prevEnd.tv_usec) / 1000;
            // printf("timeDiff:%ld,%d\n", timeDiff,i);
            write(fd,tmp,ret_size);
            data_len += ret_size;
        }

        //4-7 文件长度
        //最后4个字节，数据长度
        unsigned char wav_head[WAV_HEAD_LEN] = 
        {
            0x52,0x49,0x46,0x46,0xFC,0xFD,0xFE,0xFF,0x57,0x41,0x56,0x45,0x66,0x6d,0x74,0x20,
            0x12,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x80,0x3e,0x00,0x00,0x00,0x7d,0x00,0x00,
            0x02,0x00,0x10,0x00,0x00,0x00,0x4c,0x49,0x53,0x54,0x1a,0x00,0x00,0x00,0x49,0x4e,
            0x46,0x4f,0x49,0x53,0x46,0x54,0x0e,0x00,0x00,0x00,0x4c,0x61,0x76,0x66,0x35,0x38,
            0x2e,0x32,0x39,0x2e,0x31,0x30,0x30,0x00,0x64,0x61,0x74,0x61,0xFC,0xFD,0xFE,0xFF
        };

        wav_head[WAV_HEAD_LEN - 1] = (data_len >> 24) & 0xFF;
        wav_head[WAV_HEAD_LEN - 2] = (data_len >> 16) & 0xFF;
        wav_head[WAV_HEAD_LEN - 3] = (data_len >> 8) & 0xFF;
        wav_head[WAV_HEAD_LEN - 4] = data_len & 0XFF;

        data_len += WAV_HEAD_LEN;
        wav_head[7] = (data_len >> 24) & 0xFF;
        wav_head[6] = (data_len >> 16) & 0xFF;
        wav_head[5] = (data_len >> 8) & 0xFF;
        wav_head[4] = data_len & 0XFF;

        lseek(fd,SEEK_SET,0);
        write(fd,wav_head,WAV_HEAD_LEN);

        close(fd);

        //m_cAiInterface->unInit();
        return 0;
    }

    int CCliAgentHostT41::AudioAplay(std::string strPath)
    {
	    if(m_cAoInterface == NULL)
        {
            cli::OutputDevice::GetStdErr() << "m_cAoInterface:new failure" << cli::endl;
            return -1;
        }

        // m_cAoInterface->init();

        int fd = open(strPath.c_str(),O_RDONLY);
        if(fd <= 0)
        {
            cli::OutputDevice::GetStdErr() << "AudioAplay:open failure" << cli::endl;
            logPrint(MX_LOG_INFOR, "%d", fd);
            return -2;
        }

        if(m_cAoInterface == NULL)
        {
            cli::OutputDevice::GetStdErr() << "m_cAoInterface:new failure" << cli::endl;
            close(fd);
            return -3;
        }

        int ret_size = 0;
        unsigned char tmp[AUDIO_FRAME_LEN];

        m_cAoInterface->resumeChn();
        lseek(fd,SEEK_SET,WAV_HEAD_LEN);
        int num = 0;
        do
        {
            num++;
            ret_size = read(fd,tmp,AUDIO_FRAME_LEN);
           
            if(ret_size != AUDIO_FRAME_LEN)
            {
                break;
            }
            
            m_cAoInterface->writeFrame(0,tmp,ret_size);
           
            usleep(40*1000);

        }while(1);
        m_cAoInterface->pauseChn();
        m_cAoInterface->clearChnBuf();
        close(fd);

        // m_cAoInterface->unInit();

        return 0;
    }


// iIndex:0 白光，1 红外;iFlag:0 关，1 开
    int CCliAgentHostT41::LedSet(int iIndex,int iFlag)
    {
        std::string cmd;
        if(iIndex == 0)
        {
            if(iFlag == 1)
            {
                cmd = "/system/bin/wled on 10000 1";
            }
            else if(iFlag == 0)
            {
                cmd = "/system/bin/wled off";
            }
        }
        else if(iIndex == 1)
        {
            if(iFlag == 1)
            {
                cmd = "/system/bin/irled on";
            }
            else if(iFlag == 0)
            {
                cmd = "/system/bin/irled off";
            }
        }
        system(cmd.c_str());

        return 0;
    }

    int CCliAgentHostT41::IrCutSet(int iFlag)
    {
        std::string cmd;
        if(iFlag == 1)
        {
            cmd = "/system/bin/ircut off";
        }
        else if(iFlag == 0)
        {
            cmd = "/system/bin/ircut on";
        }
        system(cmd.c_str());

        return 0;
    }

    int CCliAgentHostT41::NightModeSet(int iFlag)
    {
        std::string cmd;
        if(iFlag == 1)
        {
            LedSet(0,0);
            cmd = "echo 0 > /tmp/dn";
            system(cmd.c_str());
            IrCutSet(1);
            LedSet(1,1);
        }
        else if(iFlag == 0)
        {
            cmd = "echo 1 > /tmp/dn";
            system(cmd.c_str());
            IrCutSet(0);
            LedSet(1,0);
        }
        
        return 0;
    }

    static std::string ReadFileToSting(const char* filename)
    {
        FILE* file = fopen(filename, "r"); 
        if (file == NULL) {
            perror("Error opening file"); 
            return "-2";
        }

        char buffer[128];

        size_t result = fread(buffer, 1, sizeof(buffer), file); 
        buffer[result] = '\0';

        fclose(file); 
        return std::string(buffer);
    }

    std::string CCliAgentHostT41::GetVersion()
    {
        INI::CINIFile iniVersionConfig;
        iniVersionConfig.load("/etc/version");
        std::string strNorVersion = iniVersionConfig["APP_FW_CONFIG"]["VERSION"].as<std::string>();
        
        iniVersionConfig.load("/system/version");
        std::string strNandVersion= iniVersionConfig["APP_FW_CONFIG"]["VERSION"].as<std::string>();

        return strNorVersion + " " + strNandVersion;
    }

    std::string CCliAgentHostT41::GetAlsValue()
    {
        // return ReadFileToSting("/sys/als/device/property/value");
        std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW als_400_adc");

		int iPos = strCmdValue.find("Value=");
		if(-1 == iPos) {
			return "-1";
		}

		std::string strAlsCriterionAdc = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
        int AlsCriterionAdc = atoi(strAlsCriterionAdc.c_str());
        if(AlsCriterionAdc == 0) {
            return "-1";
        }
        // 等待光敏稳定
        for(int i = 0; i < 10;i++) {
            ReadFileToSting("/sys/als/device/property/value");
            usleep(100 * 1000);
        }
        
        int Sum = 0;
        for (int i = 0; i < 3;i++) {
            std::string strRet = ReadFileToSting("/sys/als/device/property/value");
            int AlsAdc = atoi(strRet.c_str());
            if(AlsAdc < 0) {
                return "-1";
            }
            
            Sum += AlsAdc;
            usleep(200 * 1000);
        }
            
        float Lux = 400.0 / (float)AlsCriterionAdc * (float)Sum / 3.0;
        
        return std::to_string(Lux);
    }

    std::string CCliAgentHostT41::GetAlsRaw()
    {
        return ReadFileToSting("/sys/als/device/property/value");
    }

    std::string CCliAgentHostT41::TestAls()
    {
        return ReadFileToSting("/sys/als/device/property/interrupt");
    }

    std::string CCliAgentHostT41::CalibrationAls()
    {
        int Sum = 0;
        std::string strObjValue;

        // 等待光敏稳定
        for(int i = 0; i < 10;i++) {
            ReadFileToSting("/sys/als/device/property/value");
            usleep(100 * 1000);
        }

        for (int i = 0; i < 3;i++) {
            std::string strRet = ReadFileToSting("/sys/als/device/property/value");
            int AlsAdc = atoi(strRet.c_str());
            if(AlsAdc < 0) {
                return "-1";
            }
            
            Sum += AlsAdc;
            usleep(200 * 1000);
        }

        Sum = Sum / 3;

        linuxPopenExecCmd(strObjValue, "tag_env_info --set HW als_400_adc %d", Sum);
        setFWParaConfig("factory","als_400_adc",std::to_string(Sum).c_str(),1); 
        saveFWParaConfig();
        system("sync");
        sleep(1);
        return std::to_string(Sum);
    }
}
