#include "efuse.h"
#include <stdio.h>
#include <string.h>

int getDID()
{
	char did[DID_LEN + 1] = { 0 };
	if(readEfuse(SCB_DATA, DID_OFFSET, (unsigned char *)did, DID_LEN) == 0){
		printf("%s\n", did);
		return 0;
	} else {
		return -1;
	}
}

int getPSK()
{
	char psk[PSK_LEN + 1] = { 0 };
	if(readEfuse(USER_DATA, PSK_OFFSET, (unsigned char *)psk, PSK_LEN) == 0){
		printf("%s\n", psk);
		return 0;
	} else {
		return -1;
	}
}

int getMAC()
{
	char mac[MAC_LEN + 1 + 5] = { 0 };
    char *tmp = mac;
	if(readEfuse(SCB_DATA, MAC_OFFSET, (unsigned char *)mac, MAC_LEN) == 0){
        for(int i = 0; i < 5; i++) {
            memmove(tmp + 3,tmp + 2,strlen(tmp));
            tmp[2] = ':';
            tmp += 3;
        }
		printf("%s\n", mac);
		return 0;
	} else {
		return -1;
	}
}

int main(int argc, char **argv)
{
    if(argc == 2) {
        if(strcmp("mac",argv[1]) == 0) {
            return getMAC();
        } else if(strcmp("psk",argv[1]) == 0) {
            return getPSK();
        } else if(strcmp("did",argv[1]) == 0) {
            return getDID();
        }
    }

    return -1;
}