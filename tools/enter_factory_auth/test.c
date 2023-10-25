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
#include "openssl/err.h"
#include "openssl/sha.h"
#include "openssl/des.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "io.h"

int pub_encrypt(const char *priname, char *buf, int size, char *out, int o_size)
{
    RSA* rsa ;
    FILE *fp = NULL;

    if ((fp = fopen(priname, "r")) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    
    int rsa_len = RSA_size(rsa);
    char *en = (char *)malloc(rsa_len + 1);
    memset(en, 0, rsa_len + 1);
    printf("rsa_len:%d\n", rsa_len);

    if (RSA_public_encrypt(size, (unsigned char *)buf, (unsigned char*)en, rsa, RSA_PKCS1_PADDING) < 0) {
        printf("RSA_private_encrypt error, %s,maybe your sha greater than mod n , %s, %d\n", ERR_error_string (ERR_get_error (), (char *) buf), __func__, __LINE__);
		goto ERR;
    }

    RSA_free(rsa);
    memcpy(out, en, rsa_len);
    free(en);

    return 0;
ERR:
    RSA_free(rsa);
    memcpy(out, en, rsa_len);
    free(en);

    return -1;
}

int pri_encrypt(const char *priname, char *buf, int size, char *out, int o_size)
{
    RSA* rsa ;
    FILE *fp = NULL;

    if ((fp = fopen(priname, "r")) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    
    int rsa_len = RSA_size(rsa);
    char *en = (char *)malloc(rsa_len + 1);
    memset(en, 0, rsa_len + 1);
    printf("rsa_len:%d\n", rsa_len);

    if (RSA_private_encrypt(size, (unsigned char *)buf, (unsigned char*)en, rsa, RSA_PKCS1_PADDING) < 0) {
        printf("RSA_private_encrypt error, %s,maybe your sha greater than mod n , %s, %d\n", ERR_error_string (ERR_get_error (), (char *) buf), __func__, __LINE__);
		goto ERR;
    }

    RSA_free(rsa);
    memcpy(out, en, rsa_len);
    free(en);

    return 0;
ERR:
    RSA_free(rsa);
    memcpy(out, en, rsa_len);
    free(en);

    return -1;
}

int pri_decrypt(const char *priname, char *buf, int size, char *out, int o_size)
{
    RSA *rsa = NULL;
    FILE *fp = NULL;
    if ((fp = fopen(priname, "r")) == NULL) {
        return -1;
    }

    if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    int len = size;
    int rsa_len = RSA_size(rsa);
    printf("len: %d,rsa_len: %d\n", len, rsa_len);
    char *de = (char *)malloc(rsa_len + 1);
    memset(de, 0, rsa_len + 1);

    int ret = RSA_private_decrypt(rsa_len, (unsigned char *)buf, (unsigned char*)de, rsa, RSA_PKCS1_PADDING);
    if ( ret < 0) {
        printf("%s\n",de);
        printf("%s, %d---0x%x 0x%x\n", __func__, __LINE__,ret,-ret);
        return -1;
    }
    RSA_free(rsa);
    fclose(fp);

    memcpy(out, de, o_size);
    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 3) {
        printf("Usage:ip port\n");
        return -1;
    }

    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
 
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2])); 
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        exit(1);
    }

    char msg[256] = "{\"ts\":\"1637821232132\",\"pc_random\":\"abc\"}";
    char msg1[256] = "{\"ts\":\"1637821232139\",\"pc_random\":\"cba\",\"ipc_random\":\"321\"}";
    char ciphertext[256];
    char outbuf[256];
    pri_encrypt("rsa_private_key.pem",msg,strlen(msg),ciphertext,sizeof(ciphertext));

    send_buf(sock_cli, ciphertext,sizeof(ciphertext));
    int recv_len = recv_timeout(sock_cli,ciphertext, sizeof(ciphertext), 2000);
    printf("recv_len:%d\n",recv_len);
    if(recv_len > 0) {
        pri_decrypt("rsa_private_key.pem",ciphertext,sizeof(ciphertext),outbuf,sizeof(outbuf));
        printf("outbuf:%s\n",outbuf);

        // pub_encrypt("rsa_private_key.pem",msg,strlen(msg),ciphertext,sizeof(ciphertext));
        // pri_decrypt("rsa_private_key.pem",ciphertext,sizeof(ciphertext),outbuf,sizeof(outbuf));
        // printf("outbuf:%s\n",outbuf);
    }

    pri_encrypt("rsa_private_key.pem",msg1,strlen(msg1),ciphertext,sizeof(ciphertext));
    send_buf(sock_cli, ciphertext,sizeof(ciphertext));

    close(sock_cli);

    return 0;
}