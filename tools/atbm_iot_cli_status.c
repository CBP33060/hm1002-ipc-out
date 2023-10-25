#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <asm/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "atbm_tool.h"
#include "atbm_ioctl_ext.h"

void MAC_printf(unsigned char mac[6])
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int main(int argc, char *argv[])
{
	int fd = 0;
	int ret = 0;
	int flags = 0;
	struct status_info status_info;
	struct in_addr addr;
	struct rssi_info rssi;
	struct rate_info rate;

	if (argc != 2)
    {
        printf("unknow commond!\n");
        exit(0);
    }

	do
	{
		fd = open("/dev/atbm_ioctl", O_RDWR);
		if (fd < 0)
		{
			printf("open /dev/atbm_ioctl fail.\n");
		}
		else
		{
			break;
		}
		usleep(100*1000);
	}while(1);

	fcntl(fd, F_SETOWN, getpid());
	flags = fcntl(fd, F_GETFL); 
	fcntl(fd, F_SETFL, flags | FASYNC);
 
	if(strcmp(argv[1], "rssi") == 0)
	{
		ret = ioctl(fd, ATBM_RSSI, &rssi);
		if (ret)
		{
			goto err;
		}
		printf("%d\n", rssi.rssi);
	}
	else if(strcmp(argv[1], "freq") == 0)
	{
		ret = ioctl(fd, ATBM_TX_RATE, &rate);
		if (ret)
		{
			goto err;
		}
		printf("%d\n", rate.rate);
		// if (rate_transform2str(rate.rate) == NULL)
		// {
		// 	goto err;
		// }
		// printf("%s\n", rate_transform2str(rate.rate));
	}
	else
	{
		memset(&status_info, 0, sizeof(struct status_info));
		ret = ioctl(fd, ATBM_STATUS, (unsigned int)(&status_info));
		if (ret)
		{
			goto err;
		}

		if(strcmp(argv[1], "wifi_mode") == 0)
		{
			printf("%s\n", status_info.wifimode?"AP":"STA");
		}
		
		if (status_info.wifimode || status_info.bconnect)
		{
			if(strcmp(argv[1], "state") == 0)
			{
				printf("ACTIVE\n");
			}

			if(strcmp(argv[1], "ssid") == 0)
			{
				status_info.con_event.ssid[status_info.con_event.ssidlen] = '\0';
				printf("%s\n", status_info.con_event.ssid);
			}

			addr.s_addr = status_info.con_event.ipaddr;

			if(strcmp(argv[1], "ip_address") == 0)
			{
				printf("%s\n", inet_ntoa(addr));
			}

			if(strcmp(argv[1], "mac_address") == 0)
			{
				//printf("mac_address=");
				if (status_info.wifimode)
				{
					MAC_printf(status_info.macaddr);
				}
				else
				{
					MAC_printf(status_info.con_event.bssid);
				}
				printf("\n");
			}

			if(strcmp(argv[1], "bssid") == 0)
			{
				MAC_printf(status_info.con_event.bssid);
				printf("\n");
			}

			addr.s_addr = status_info.con_event.ipmask;
			if(strcmp(argv[1], "ip_mask") == 0)
			{
				printf("%s\n", inet_ntoa(addr));
			}

			addr.s_addr = status_info.con_event.gwaddr;
			if(strcmp(argv[1], "gate_way") == 0)
			{
				printf("%s\n", inet_ntoa(addr));
			}
		}
		else
		{
			if(strcmp(argv[1], "state") == 0)
			{
				printf("INACTIVE\n");
			}
		}
	}
 
	return 0;
err:
	return -1;
}
