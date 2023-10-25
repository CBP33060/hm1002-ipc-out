#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

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
#include "crypt_api_mx.h"

void InitLog()
{
    maix::T_LogConfig tLogConfig;
    tLogConfig.m_strName = "MXFactory";
    tLogConfig.m_eLevel = maix::MX_LOG_TRACE;
    tLogConfig.m_eType = maix::MX_LOG_CONSOLE_AND_LOCAL;
    tLogConfig.m_strFileName = "";
    tLogConfig.m_eNetType = maix::MX_LOG_TCP;
    tLogConfig.m_strIP = "";
    tLogConfig.m_iPort = 0;
    tLogConfig.m_strUnix = "/tmp/70mai/log_manage_epoll_server";
    maix::logInit(tLogConfig);
}

void InitMcu()
{
    maix::T_uMcuIoConfig IoConfig;

    IoConfig.sUartConfig.iBaudRate = 115200;
    IoConfig.sUartConfig.iBits = 8;
    IoConfig.sUartConfig.cParity = 'N';
    IoConfig.sUartConfig.iStop = 1;
    IoConfig.sUartConfig.iInterval = 10; // ms,超过这个时间没有接收到数据代表这一个串口数据报就接收完了；
    strcpy(IoConfig.sUartConfig.strUartPath,UART_PATH);
    
    maix::CCliAgentMcuIoDevice * CliAgentMcuIo = new maix::CCliAgentMcuIoUart();
    CliAgentMcuIo->Configure(IoConfig);
    maix::CCliAgentMcu *CliAgentMcu = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);
    CliAgentMcu->SetIO(CliAgentMcuIo);
}

void InitHost()
{
    maix::CCliAgentHost *CliAgentHost = maix::CCliAgentHostFactory::GetInstance(HOST_NAME);
}

void InitCliDebug()
{
    // static const cli::TraceClass CLI_CLI("CLI_CLI", cli::Help());
    // static const cli::TraceClass CLI_SHELL("CLI_SHELL", cli::Help());
    // static const cli::TraceClass CLI_EXEC_CTX("CLI_EXEC_CTX", cli::Help());
    // static const cli::TraceClass CLI_EXECUTION("CLI_EXECUTION", cli::Help());
    // static const cli::TraceClass CLI_NCURSES_CONSOLE("CLI_NCURSES_CONSOLE", cli::Help());
    // static const cli::TraceClass CLI_TELNET_SERVER("CLI_TELNET_SERVER", cli::Help());
    // class _Trace { public:
    //     explicit _Trace() {
    //         cli::GetTraces().SetStream(cli::OutputDevice::GetStdErr());
    //         cli::GetTraces().Declare(CLI_CLI);        cli::GetTraces().SetFilter(CLI_CLI, false);
    //         cli::GetTraces().Declare(CLI_SHELL);       cli::GetTraces().SetFilter(CLI_SHELL, false);
    //         cli::GetTraces().Declare(CLI_EXEC_CTX);         cli::GetTraces().SetFilter(CLI_EXEC_CTX, false);
    //         cli::GetTraces().Declare(CLI_EXECUTION);         cli::GetTraces().SetFilter(CLI_EXECUTION, false);
    //         cli::GetTraces().Declare(CLI_NCURSES_CONSOLE);         cli::GetTraces().SetFilter(CLI_NCURSES_CONSOLE, false);
    //         cli::GetTraces().Declare(CLI_TELNET_SERVER);         cli::GetTraces().SetFilter(CLI_TELNET_SERVER, false);
    //     }
    //     ~_Trace() {
    //         cli::GetTraces().UnsetStream(cli::OutputDevice::GetStdErr());
    //     }
    // } guard;
}

/**
 senv;[HW];init_vw=2560;init_vh=1440;nrvbs=1;mode=0;ldc_mode=0;user_wdr_mode=0;70mai_factory_mode=0;70mai_system_partition=0;70mai_wm=1;70mai_dn_mode=1;70mai_dn_sw=0;70mai_wdr=0;pwm_duty=2500:2500;adc_value=100;wl_mode=off;ir_mode=off;als_400_adc=45000;hx3205_count=3;[OTA];ota_step=0;eenv; lzo_size=4133663 rd_start=0x80800000 rd_size=0xb5f600 os2_flag=0x0 tagbk=0x5  riscv_fw=0 sensor_setting_fw=0
 */
void CheckEnv()
{
    int tagbk = 0;
    int fd;
    char buffer[1024]={0};
	memset(buffer, 0, sizeof(buffer));
    if((fd = open("/proc/cmdline", O_RDONLY)) < 0)
    {
		printf("open /proc/cmdline error %d, %s\r\n", errno,strerror(errno));
        return;
    }
    int size = read(fd,buffer,sizeof(buffer));
    if(size <= 0)
    {
        close(fd);
		printf("open /proc/cmdline error size = 0\r\n");
        return;
    }
    printf("buffer:%s\n",buffer);
    char *work = NULL;
    if ((work = strstr(buffer, "tagbk="))!= NULL)
    {
        sscanf(work, "tagbk=0x%d", &tagbk); 
		printf("tagbk tagbk=[%d]\r\n", tagbk);
    }

    if(tagbk != 0)
    {
        char cmd[1024];
        char env_buf[4096] = {0};
        mtd_info_t mtd_info;
        int mtd_fd = open("/dev/mtdblock1",O_RDWR);

        //env bak
        lseek(mtd_fd, 0xF000, SEEK_SET);

        int ret = read(mtd_fd, env_buf, 4096);
        if (ret != 4096) {
            printf("write tag failed\n");
            return;
        }

        printf("env_buf[%d]:%s\n",strlen(env_buf),env_buf);

        lseek(mtd_fd, 0x7000, SEEK_SET);

        ret = write(mtd_fd, env_buf, 4096);
        if (ret != 4096) {
            printf("write tag failed\n");
            return;
        }

        char *buf = maix::getFWParaConfig("user","set_wdr_mode");
        int val = 0;
        if(buf != NULL)
        {
            val = atoi(buf);
            sprintf(cmd,"tag_env_info --set HW 70mai_wdr %d",val);
            printf("cmd:%s\n",cmd);
            system(cmd);
        }

        buf = maix::getFWParaConfig("user","set_water_mark");
        val = 0;
        if(buf != NULL)
        {
            val = atoi(buf);
            sprintf(cmd,"tag_env_info --set HW 70mai_wm %d",val);
            printf("cmd:%s\n",cmd);
            system(cmd);
        }

        buf = maix::getFWParaConfig("user","set_night_shot");
        val = 0;
        if(buf != NULL)
        {
            val = atoi(buf);
            sprintf(cmd,"tag_env_info --set HW 70mai_dn_sw %d",val);
            printf("cmd:%s\n",cmd);
            system(cmd);
        }
        std::string strResult;
        if(val == 0)
		{
			linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode off");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode off");
		}
		else if(val == 1)
		{
			linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode off");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode auto");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW pwm_duty 1900:3100");
		}
		else if(val == 2)
		{
			linuxPopenExecCmd(strResult, "tag_env_info --get HW white_light_brightness");
			int iPos = strResult.find("Value=");
			int iValue = 50;
			if(-1 == iPos)
			{
				char *cValue = getFWParaConfig("white_light_brightness");
				if(cValue == NULL)
				{
					linuxPopenExecCmd(strResult, "tag_env_info --set HW white_light_brightness 50");
					setFWParaConfig("white_light_brightness","50",1);
				}
				else
				{
					iValue = atoi(cValue);
					linuxPopenExecCmd(strResult, "tag_env_info --set HW white_light_brightness %d",iValue);
				}
			}
			linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode auto");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode off");
			// 0-100映射到0-50，灯亮度调整之后要修改
            if(iValue != 1 )
			    iValue = iValue/2;
			linuxPopenExecCmd(strResult, "tag_env_info --set HW pwm_duty %d:%d", (iValue * 50), (5000 - iValue * 50));
		}

        buf = maix::getFWParaConfig("factory","als_400_adc");
        val = 0;
        if(buf != NULL)
        {
            val = atoi(buf);
            if(val == 45000 || val == 44999)
                sprintf(cmd,"tag_env_info --set HW als_400_adc 24999");
            else
                sprintf(cmd,"tag_env_info --set HW als_400_adc %d",val);
            printf("cmd:%s\n",cmd);
            system(cmd);
        }

        buf = maix::getFWParaConfig("70mai_system_partition");
        val = 0;
        if(buf != NULL)
        {
            val = atoi(buf);
            linuxPopenExecCmd(strResult, "tag_env_info --set HW 70mai_system_partition %d", val);
        }

        linuxPopenExecCmd(strResult, "tag_env_info --set HW adc_value 2");
        sprintf(cmd,"tag_env_info --set HW 70mai_factory_mode 0");
        printf("cmd:%s\n",cmd);
        system(cmd);

        system("reboot");
    }
}

int main(int I_Args, const char* ARSTR_Args[])
{
    CheckEnv();
    verify_boot();
    InitLog();
    InitMcu();
    InitHost();

    system("echo 10 > /sys/als/device/property/interrupt");
    
    std::string mcuflag;
    linuxPopenExecCmd(mcuflag, "tag_env_info --get HW 70mai_factory_mode");
    int ret = mcuflag.find("Value=2") ;
    if(ret > 0)
    {
        CREATE_MCU_INSTANCE
        CliAgentMcu->MCUUpgrade();
    }

    BurnInHandle();

    SampleArgs CliArgs;
    if (! CliArgs.ParseArgs(I_Args, ARSTR_Args)) { 
        return -1;
    }

    if (CliArgs.GetInput() == NULL && CliArgs.IsTelnet() == false) {
        CliArgs.DisplayHelp(ARSTR_Args[0]);
        return -1;
    }

    cli_agent_client CliAgentClient;
    cli::Shell CliShell(CliAgentClient);

    InitCliDebug();

    CliArgs.Execute(CliShell);

    return 0;
}
