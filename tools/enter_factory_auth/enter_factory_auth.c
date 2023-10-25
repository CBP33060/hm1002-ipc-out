#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <ctype.h>
#include <errno.h>
#include "io.h"
#include "cJSON.h"

#include <mbedtls/sha256.h>
#include <mbedtls/rsa.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#define BIG_LITTLE_SWAP32(x)        ( (((*(long int *)&x) & 0xff000000) >> 24) | \
                                      (((*(long int *)&x) & 0x00ff0000) >> 8) | \
                                      (((*(long int *)&x) & 0x0000ff00) << 8) | \
                                      (((*(long int *)&x) & 0x000000ff) << 24) )

/*
    0x00000 - 0x2FE00 uboot
    0x2FE00 - 0x2FF00 uboot区域的被hash，hash值被私钥加密后的数据 256 byte
    0x2FF00 - 0x30000 RSA MODEL N 公钥模数N  256 byte
*/
#define UBOOT_SIZE (48 * 1024)
#define RSA_E   "10001"

mbedtls_rsa_context g_rsa;
const uint8_t g_ctr_drbg_byte[] = "rsa sample";
mbedtls_entropy_context g_entropy;
mbedtls_ctr_drbg_context g_ctr_drbg;

int load_public_key(mbedtls_rsa_context *rsa)
{
    static unsigned char buf[UBOOT_SIZE];
    int ret = 0;
    unsigned char  module[256] = {0};
    int fd = open("/dev/mtd0",O_RDONLY);
    if(fd < 0) {
        printf("open failed for uboot\n");
        return -1;
    }
    int len = read(fd,buf,sizeof(buf));
    if(len != UBOOT_SIZE) {
        printf("Error reading boot\n");
        return -1;
    }
    close(fd);

    memcpy(module,buf + 0x300,256);

    uint32_t *module_32 = (uint32_t *)module;
	for (int i = 0; i < 256/4; i++) {
		module_32[i] = BIG_LITTLE_SWAP32(module_32[i]);
	}

    mbedtls_mpi K;
    mbedtls_mpi_init( &K );
    mbedtls_rsa_init( rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_string( &K, 16, RSA_E  ) );
    MBEDTLS_MPI_CHK( mbedtls_rsa_import( rsa, NULL, NULL, NULL, NULL, &K ) );
    MBEDTLS_MPI_CHK( mbedtls_rsa_import_raw(rsa,module,256,NULL,0,NULL,0,NULL,0,NULL,0) );
    MBEDTLS_MPI_CHK( mbedtls_rsa_complete( rsa ) );
    if( mbedtls_rsa_check_pubkey(  rsa ) != 0 ) {
        printf( "mbedtls_rsa_check_pubkey failed\n" );
        ret = -2;
        goto cleanup;
    }

cleanup:
    mbedtls_mpi_free( &K );
    
    return ret;
}

void reverse_str(char* s, int size){
	
	int start = 0;
	int end   = size - 1;
	char tempchar = 0;
	
	if(size == 0 || size == 1)
		s = s;

	while(start < end){
		tempchar = s[start];
		s[start] = s[end];
		s[end]   = tempchar;
		++start;
		--end;
	}
}

char* gen_random_str(char *string,int length)
{
    unsigned int flag, i;
    int fd = open("/dev/urandom",O_RDONLY);
    if(fd < 0)
        srand((unsigned) time(NULL ));
    for (i = 0; i < length-1; i++)
    {
        if(fd < 0)
            flag = rand();
        else 
            read(fd,&flag,sizeof(flag));
        flag %= 3;

        switch (flag)
        {
            case 0:
                if(fd < 0)
                    flag = rand();
                else
                    read(fd,&flag,sizeof(flag));
                string[i] = 'A' + flag % 26;
                break;
            case 1:
                if(fd < 0)
                    flag = rand();
                else
                    read(fd,&flag,sizeof(flag));
                string[i] = 'a' + flag % 26;
                break;
            case 2:
                if(fd < 0)
                    flag = rand();
                else
                    read(fd,&flag,sizeof(flag));
                string[i] = '0' + flag % 10;
                break;
            default:
                string[i] = 'x';
                break;
        }
    }
    string[length - 1] = '\0';
    if(fd > 0)
        close(fd);
    return string;
}

int auth_process(int fd,char *buf,int len)
{
    static int s_ts[128] = {0};
    static char s_pc_random[128] = {0};
    static char s_local_random[128] = "123";
    static int state = 0;
    
    char rsa_decrypted[256];
    char rsa_outbuf[256];
    int ret = mbedtls_rsa_pkcs1_decrypt( &g_rsa, rand, NULL, MBEDTLS_RSA_PUBLIC,
                                    &len, buf, rsa_decrypted,
                                    sizeof(rsa_decrypted) );
    if(ret < 0) {
        printf("mbedtls_rsa_pkcs1_decrypt err:0x%x 0x%x\n",ret,-ret);
        return -1;
    }

    printf("rsa_decrypted[%d]:%s\n",len,rsa_decrypted);

    switch (state)
    {
        case 0:{
            cJSON *jsonRoot = cJSON_Parse(rsa_decrypted);
            if (jsonRoot) {
                cJSON *jsontmp = cJSON_GetObjectItem(jsonRoot, "ts");
                if (jsontmp)
                    strcpy(s_ts, jsontmp->valuestring);
                else
                    return -1;

                jsontmp= cJSON_GetObjectItem(jsonRoot, "pc_random");
                if (jsontmp)
                    strcpy(s_pc_random, jsontmp->valuestring);
                else
                    return -1;

                cJSON_Delete(jsonRoot);
            }

            reverse_str(s_pc_random,strlen(s_pc_random));
            gen_random_str(s_local_random,9);

            cJSON *jsonRoot_1 = cJSON_CreateObject();
            cJSON_AddStringToObject(jsonRoot_1, "ts", s_ts);
            cJSON_AddStringToObject(jsonRoot_1, "pc_random", s_pc_random);
            cJSON_AddStringToObject(jsonRoot_1, "ipc_random", s_local_random);
            char *out = cJSON_Print(jsonRoot_1);
            printf("out[%d]:%s\n",strlen(out), out);
            char rsa_buf[256] = { 0 };
            strcpy(rsa_buf,out);
            ret = mbedtls_rsa_pkcs1_encrypt(  &g_rsa, mbedtls_ctr_drbg_random, &g_ctr_drbg, MBEDTLS_RSA_PUBLIC, 
                        strlen(out), out,rsa_outbuf );

            cJSON_Delete(jsonRoot_1);
            if (out)
                free(out);
            if(ret < 0) {
                printf("mbedtls_rsa_pkcs1_encrypt err:0x%x 0x%x\n",ret,-ret);
                return -1;
            }
            send_buf(fd,rsa_outbuf,256);
            state++;
        }
            break;
        case 1:{
            state=0;
            static int ts[128] = {0};
            static char pc_random[128] = {0};
            static char local_random[128] = {0};
            cJSON *jsonRoot = cJSON_Parse(rsa_decrypted);
            if (jsonRoot) {
                cJSON *jsontmp = cJSON_GetObjectItem(jsonRoot, "ts");
                if (jsontmp)
                    strcpy(ts, jsontmp->valuestring);
                else
                    return -1;

                jsontmp= cJSON_GetObjectItem(jsonRoot, "pc_random");
                if (jsontmp)
                    strcpy(pc_random, jsontmp->valuestring);
                else
                    return -1;
                
                jsontmp= cJSON_GetObjectItem(jsonRoot, "ipc_random");
                if (jsontmp)
                    strcpy(local_random, jsontmp->valuestring);
                else
                    return -1;

                cJSON_Delete(jsonRoot);
            }
            long long int ts1 = atoll(ts) - 5;
            long long int  ts2 = atoll(s_ts);

            reverse_str(local_random,strlen(local_random));
            printf("ts1 = %lld, ts2 = %lld, local_random:%s, s_local_random:%s,pc_random:%s s_pc_random:%s\n", ts1, ts2,local_random,s_local_random,pc_random,s_pc_random);
            if(ts1 > ts2 || strcmp(pc_random,s_pc_random) != 0 || strcmp(local_random,s_local_random) != 0 ) {
                printf("failed\n");
                return -1;
            } else {
                printf("successful\n");
                system("tag_env_info --set HW 70mai_factory_mode 1");
                system("reboot");
            }
        }
            break;
        default:
            break;
    }

    return 0;
}

int main(int argc, char **argv)
{ 
    int ret = load_public_key(&g_rsa);
    mbedtls_entropy_init(&g_entropy);
    mbedtls_ctr_drbg_init(&g_ctr_drbg);

    mbedtls_ctr_drbg_seed(&g_ctr_drbg, mbedtls_entropy_func, &g_entropy,
        g_ctr_drbg_byte, strlen((const char*)g_ctr_drbg_byte));

    int server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
 
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(7777); 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(server_sockfd, 5);

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    char buffer[256];

    // while(1) {
    #ifdef USE_SOCKET
    fd_set rset;
    struct timeval t;

    t.tv_sec = 30;
    t.tv_usec = 0;
    FD_ZERO(&rset);
    FD_SET(server_sockfd, &rset);
    ret = select(server_sockfd+1, &rset, NULL, NULL, &t);
    if (ret <= 0) 
        return -1;
    int fd = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
    if(fd<0)
    {
        perror("connect");
        exit(1);
    }
    printf("Connect \n");
    #else
        int fd = open("/dev/ttyGS0", O_RDWR);
        if(fd<0){
            perror("open failed");
            return -1;
        }
        if(set_uart_opt(fd,8,'N',115200,1) < 0) {
            return -1;
        }
    #endif
    while(1) {
        memset(buffer,0,sizeof(buffer));
        int len = recv_timeout(fd, buffer, sizeof(buffer),5000);
        if(len <= 0){
            return -1;
        }

        if(auth_process(fd,buffer,len) < 0) {
            return -1;
        }
    }
    printf("Disconnect\n");
    close(fd);
    // }

    close(server_sockfd);
    mbedtls_rsa_free( &g_rsa );

    return 0;
}