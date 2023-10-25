#ifndef __CRYPT_API_H__
#define __CRYPT_API_H__
#include "global_export.h"
#include "type_def.h"
#include <string>
#include <vector>

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#define JZAES_IOC_MAGIC  'A'
#define IOCTL_AES_GET_PARA					_IOW(JZAES_IOC_MAGIC, 110, unsigned int)
#define IOCTL_AES_START_EN_PROCESSING		_IOW(JZAES_IOC_MAGIC, 111, unsigned int)
#define IOCTL_AES_START_DE_PROCESSING		_IOW(JZAES_IOC_MAGIC, 112, unsigned int)



typedef enum jz_aes_status {
	JZ_AES_STATUS_PREPARE = 0,
	JZ_AES_STATUS_WORKING,
	JZ_AES_STATUS_DONE,
} JZ_AES_STATUS;

typedef enum IN_UNF_CIPHER_WORK_MODE_E
{
	IN_UNF_CIPHER_WORK_MODE_ECB = 0x0,
	IN_UNF_CIPHER_WORK_MODE_CBC = 0x1,
	IN_UNF_CIPHER_WORK_MODE_OTHER = 0x2
}IN_UNF_CIPHER_WORK_MODE;

struct aes_para {
	unsigned int status;
	unsigned int enworkmode;
	unsigned int aeskey[4];
	unsigned int aesiv[4]; // when work mode is cbc, the parameter must be setted.
	unsigned char *src;
	unsigned char *dst;
	unsigned int datalen; // it should be 16bytes aligned
	unsigned int donelen; // The length of the processed data.
};

MAIX_EXPORT int crypto_ecdh_gen_public(std::string &strPubKey,
	std::string &strPriKey);

MAIX_EXPORT int crypto_ecdh_compute_shared(std::string strPubkey,
	std::string strPriKey, unsigned char* pcAESKey);

MAIX_EXPORT int crypto_aes128_encrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len);

MAIX_EXPORT int crypto_aes128_encrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* base64_output_buffer, 
	int base64_output_buffer_len,
	size_t *base64_output_len);

MAIX_EXPORT int crypto_aes128_decrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len);

MAIX_EXPORT int crypto_aes128_decrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len);

MAIX_EXPORT int crypto_sha256_calcu(
	const unsigned char *input, size_t len,
	unsigned char *output);

MAIX_EXPORT int crypto_hmac_sha256(std::string strKey,
	std::string strDID, std::string strMac, unsigned char *hmac);
	
MAIX_EXPORT int crypto_aes128_encrypt_hardware(unsigned int key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len);

MAIX_EXPORT int crypto_aes128_decrypt_hardware(unsigned int key,
	unsigned char* input, int len,
	unsigned char* out, int *output_len);
MAIX_EXPORT void crypto_aes128_init();
MAIX_EXPORT void crypto_aes128_uninit();
MAIX_EXPORT int aes_pkcs5_padding(unsigned char *in, int in_len, unsigned char **out, int *out_len);
MAIX_EXPORT int aes_pkcs5_unpadding(unsigned char *in, int in_len, unsigned char *out, int *out_len);

MAIX_EXPORT int mx_crypto_aes_cbc_encrypt(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len);
MAIX_EXPORT int mx_crypto_aes_cbc_decrypt(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len);
MAIX_EXPORT int mx_crypto_aes_cbc_encrypt_base64(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len);
MAIX_EXPORT int mx_crypto_aes_cbc_decrypt_base64(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len);
MAIX_EXPORT int crypto_aes256_encrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len);

MAIX_EXPORT int crypto_aes256_encrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* base64_output_buffer,
	int base64_output_buffer_len,
	size_t *base64_output_len);

MAIX_EXPORT int crypto_aes256_decrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* out, int *output_len);

MAIX_EXPORT int crypto_aes256_decrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len);

MAIX_EXPORT std::string crypto_sha256_base64(std::string strAuthInfo);

MAIX_EXPORT int verify_boot();

#endif //__CRYPT_API_H__
