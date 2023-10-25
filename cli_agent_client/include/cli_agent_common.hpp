#ifndef __CLI_AGENT_COMMON_HPP__
#define __CLI_AGENT_COMMON_HPP__
#include "cli/pch.h"
#include "cli/common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>  
#include <errno.h>  
#include <arpa/inet.h>  
#include <signal.h>
#include <stdarg.h>
#include <fstream>

#define UART_PATH "/dev/ttyS2"
#define MCU_TYPE  "atbm6441"

#if HOST_t41
	#define HOST_NAME  "t41"
#elif HOST_mt7628
	#define HOST_NAME  "mt7628"
#else
	#error "HOST not supported"
#endif 

#define CREATE_MCU_INSTANCE maix::CCliAgentMcu *CliAgentMcu = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);
#define CREATE_MCU_INSTANCE_LED maix::CCliAgentMcu *CliAgentMcuLed = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);
#define CREATE_HOST_INSTANCE maix::CCliAgentHost *CliAgentHost = maix::CCliAgentHostFactory::GetInstance(HOST_NAME);

#define AUDIO_FRAME_LEN 1280  // 一帧1280个字节
#define AUDIO_FRAME_NUM 75    // 1S 25帧，存3S
#define WAV_HEAD_LEN    0x50  // wav 头部信息长度

struct s_burn_in_info {
    int burn_in_total_runtime;
    int burn_in_runtime;
    int burn_in_abnormal_reboot_times;
    int als_oepn_failure_times;
    int als_read_failure_times;
    int camera_open_times;
    int camera_open_failure_times;
    int camera_read_failure_times;
    int usb_pull_out;
    int no_charge;

    int burn_in_result;
};

int parseNumber(const std::string& str);
extern void GetBurnInInfo(struct s_burn_in_info *info);
extern void BurnInHandle();

#endif //__CLI_AGENT_COMMON_HPP__
