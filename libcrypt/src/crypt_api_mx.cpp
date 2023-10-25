#include "crypt_api_mx.h"
#include "mbedtls/pk.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/base64.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/hkdf.h"
#include "mbedtls/sha256.h"
#include "mbedtls/error.h"
#include <string.h>
#include <thread>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define AES_128_LEN 16
#define AES_256_LEN 16
static const char *g_static_key = "maix";
static int fd_aes;
static bool isInited=0;
static uint8_t IV[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};


int crypto_ecdh_gen_public(std::string &strPubKey, std::string &strPriKey)
{
	int ret;
	unsigned char acPubKey[128] = { 0 };
	size_t iPubKeyLen = 0;
	unsigned char acKey[128] = { 0 };
	size_t iKeyLen = 0;
	const char *pers = "maix";
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ecp_group grp;
	mbedtls_ecp_point public_key;
	mbedtls_mpi private_key;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_ecp_group_init(&grp);
	mbedtls_ecp_point_init(&public_key);
	mbedtls_mpi_init(&private_key);

	ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
		(const unsigned char *)pers,
		strlen(pers));
	if (ret != 0)
		goto crypto_ecdh_gen_public_error;

	ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
	if (ret != 0)
		goto crypto_ecdh_gen_public_error;

	ret = mbedtls_ecdh_gen_public(&grp, &private_key, &public_key,
		mbedtls_ctr_drbg_random, &ctr_drbg);
	if (ret != 0) {
		goto crypto_ecdh_gen_public_error;
	}

	ret = mbedtls_ecp_point_write_binary(&grp, &public_key,
		MBEDTLS_ECP_PF_UNCOMPRESSED, &iKeyLen, acKey, sizeof(acKey));
	if (ret != 0)
		goto crypto_ecdh_gen_public_error;

	ret = mbedtls_base64_encode(acPubKey, sizeof(acPubKey),
		&iPubKeyLen, acKey, iKeyLen);
	if (ret != 0)
		goto crypto_ecdh_gen_public_error;

	strPubKey = std::string((char*)acPubKey, iPubKeyLen);
	memset(acKey, 0, sizeof(acKey));
	ret = mbedtls_mpi_write_string(&private_key, 16,
		(char*)acKey, sizeof(acKey), &iKeyLen);
	if (ret != 0)
		goto crypto_ecdh_gen_public_error;

	strPriKey = std::string((char*)acKey, iKeyLen);

crypto_ecdh_gen_public_error:
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	mbedtls_ecp_group_free(&grp);
	mbedtls_mpi_free(&private_key);
	mbedtls_ecp_point_free(&public_key);

	return ret;
}

int crypto_ecdh_compute_shared(std::string strPubkey,
	std::string strPriKey, unsigned char* pcAESKey)
{
	if (!pcAESKey)
		return -1;

	int ret;
	unsigned char acKey[128] = { 0 };
	size_t iKeyLen = 0;
	const char *pers = "maix";
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ecp_group grp;
	mbedtls_ecp_point public_key;
	mbedtls_mpi private_key;
	mbedtls_mpi secret;
	std::string strSecret;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_ecp_group_init(&grp);
	mbedtls_ecp_point_init(&public_key);
	mbedtls_mpi_init(&private_key);
	mbedtls_mpi_init(&secret);

	ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
		(const unsigned char *)pers,
		strlen(pers));
	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;

	ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;

	ret = mbedtls_base64_decode(acKey, sizeof(acKey),
		&iKeyLen, (unsigned char*)strPubkey.c_str(), strPubkey.length());
	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;

	ret = mbedtls_ecp_point_read_binary(&grp, &public_key,
		acKey, iKeyLen);
	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;

	ret = mbedtls_mpi_read_string(&private_key, 16,
		strPriKey.c_str());
	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;

	ret = mbedtls_ecdh_compute_shared(&grp, &secret, 
		&public_key, &private_key, 
		mbedtls_ctr_drbg_random, &ctr_drbg);

	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;


	memset(acKey, 0, sizeof(acKey));
	iKeyLen = 0;
	ret = mbedtls_mpi_write_string(&secret, 16,
		(char*)acKey, sizeof(acKey), &iKeyLen);
	if (ret != 0)
		goto crypto_ecdh_compute_shared_error;

	strSecret = std::string((char*)acKey, iKeyLen);
	memset(acKey, 0, sizeof(acKey));

	ret = mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
		(unsigned char*)g_static_key, strlen(g_static_key),
		(unsigned char*)strSecret.c_str(), strSecret.length(),
		(unsigned char*)"miot-camera", strlen("miot-camera"),
		pcAESKey,
		16);


crypto_ecdh_compute_shared_error:

	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	mbedtls_ecp_group_free(&grp);
	mbedtls_mpi_free(&private_key);
	mbedtls_ecp_point_free(&public_key);
	mbedtls_mpi_free(&secret);

	return ret;
}

int crypto_aes128_encrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len)
{
	int ret = -1;
	int index = 0;
	unsigned char input_buf[AES_128_LEN] = { 0 };
	unsigned char output_buf[AES_128_LEN] = { 0 };

	mbedtls_aes_context aesContext;
	mbedtls_aes_init(&aesContext);

	ret = mbedtls_aes_setkey_enc(&aesContext, key, 128);
	if (ret != 0)
	{
		return ret;
	}

	for (index = 0; index <= len; index += AES_128_LEN)
	{
		memset(input_buf, 0, AES_128_LEN);
		memset(output_buf, 0, AES_128_LEN);

		if (index == len)
		{
			memset(input_buf, AES_128_LEN, AES_128_LEN);
		}
		else if (index + AES_128_LEN <= len)
		{
			memcpy(input_buf, input + index, AES_128_LEN);
		}
		else
		{
			memcpy(input_buf, input + index, len % AES_128_LEN);
			memset(input_buf + len % AES_128_LEN,
				AES_128_LEN - len % AES_128_LEN, AES_128_LEN - len % AES_128_LEN);
		}

		ret = mbedtls_aes_crypt_ecb(&aesContext,
			MBEDTLS_AES_ENCRYPT, input_buf, output_buf);

		if (ret != 0)
		{
			return ret;
		}

		memcpy(output + index, output_buf, AES_128_LEN);
	}

	*output_len = index;
	return ret;
}

int crypto_aes128_encrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* base64_output_buffer,
	int base64_output_buffer_len,
	size_t *base64_output_len)
{
	unsigned char* output = (unsigned char*)malloc(len + 32);
	if (output == NULL)
		return -1;

	int output_len = 0;
	int ret = 0;
	ret = crypto_aes128_encrypt(key,
		input, len, output, &output_len);

	if (ret != 0)
	{
		free(output);
		return ret;
	}

	ret = mbedtls_base64_encode(base64_output_buffer,
		base64_output_buffer_len,
		base64_output_len,
		output, output_len);
	free(output);

	return ret;
}
int crypto_aes128_decrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* out, int *output_len)
{
	int ret = -1;
	int index = 0;
	unsigned char output_buf[16];
	char last_pending = '\0';

	if (input == NULL || out == NULL)
	{
		return -1;
	}
	mbedtls_aes_context aesContext;
	mbedtls_aes_init(&aesContext);

	ret = mbedtls_aes_setkey_dec(&aesContext, key, 128);
	if (ret != 0)
	{
		return ret;
	}

	for (index = 0; index < len; index += AES_128_LEN)
	{
		ret = mbedtls_aes_crypt_ecb(&aesContext,
			MBEDTLS_AES_DECRYPT, input + index,
			output_buf);
		if (ret != 0)
		{
			return ret;
		}

		memcpy(out + index, output_buf, AES_128_LEN * sizeof(char));
	}

	last_pending = out[index - 1];

	int i = 0;
	for (i = 1; i < AES_128_LEN; i++)
	{
		if (out[index - 1 - i] != last_pending)
			break;
	}
	if ((int)last_pending == i)
	{
		index = index - i;
	}

	out[index] = '\0';
	*output_len = index;
	return ret;
}

int crypto_aes128_decrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len)
{
	int ret = 0;
	if (key == NULL || input == NULL || output == NULL
		|| len <= 0)
		return -1;

	size_t decode_len = 0;

	if (strstr((const char *)input, "=="))
		decode_len = len / 4 * 3 - 2;
	else if (strstr((const char *)input, "="))
		decode_len = len / 4 * 3 - 1;
	else
		decode_len = len / 4 * 3;

	unsigned char *decode_input = (unsigned char *)malloc(decode_len);
	if (decode_input == NULL)
		return -1;
	memset(decode_input, 0, decode_len);
	size_t decode_out_len = 0;
	ret = mbedtls_base64_decode(decode_input,
		decode_len,
		&decode_out_len,
		input, len);

	if (ret != 0)
	{
		free(decode_input);
		return ret;
	}

	ret = crypto_aes128_decrypt(key,
		decode_input, decode_out_len,
		output, output_len);
	free(decode_input);
	return ret;
}

int crypto_aes128_encrypt_hardware(unsigned int key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len)
{


	int ret = -1;
	int index = 0;
	
	unsigned char input_buf[AES_128_LEN] = { 0 };
	unsigned char output_buf[AES_128_LEN] = { 0 };
	struct aes_para para;

	memset(&para, 0, sizeof(para));
	for(int i = 0; i < 4; i++){
		para.aeskey[i] = key;
	}


	for(index = 0; index <= len; index += AES_128_LEN)
	{
		memset(input_buf, 0, AES_128_LEN);
		memset(output_buf, 0, AES_128_LEN);
		if(index == len)
		{
			memset(input_buf, AES_128_LEN, AES_128_LEN);
		}
		else if(index + AES_128_LEN <= len)
		{
			memcpy(input_buf, input + index, AES_128_LEN);
		}
		else{
			memcpy(input_buf, input + index, len % AES_128_LEN);
			memset(input_buf + len % AES_128_LEN, 
				AES_128_LEN - len % AES_128_LEN, AES_128_LEN - len % AES_128_LEN);			
		}

		para.enworkmode = IN_UNF_CIPHER_WORK_MODE_ECB;
		para.src = input_buf;
		para.dst = output_buf;
		para.datalen = AES_128_LEN;
		para.status = 0;

		ret = ioctl(fd_aes, IOCTL_AES_START_EN_PROCESSING, &para);
		if(ret < 0)
		{
			return ret;
		}
		printf("\n");
		memcpy(output + index, para.dst, AES_128_LEN);
	}
	*output_len = index;
	return ret;

}
int crypto_aes128_decrypt_hardware(unsigned int key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len)
{
	char last_pending = '\0';
	int ret = -1;
	int index = 0;
	unsigned char input_buf[AES_128_LEN] = { 0 };
	unsigned char output_buf[AES_128_LEN] = { 0 };
	struct aes_para para;

	memset(&para, 0, sizeof(para));
	for(int i = 0; i < 4; i++){
		para.aeskey[i] = key;
	}


	for(index = 0; index < len; index += AES_128_LEN)
	{
		memset(input_buf, 0, AES_128_LEN);
		memset(output_buf, 0, AES_128_LEN);
		if(index == len)
		{
			memset(input_buf, AES_128_LEN, AES_128_LEN);
		}
		else if(index + AES_128_LEN <= len)
		{
			memcpy(input_buf, input + index, AES_128_LEN);
		}
		else{
			memcpy(input_buf, input + index, len % AES_128_LEN);
			memset(input_buf + len % AES_128_LEN, 
				AES_128_LEN - len % AES_128_LEN, AES_128_LEN - len % AES_128_LEN);			
		}
		para.enworkmode = IN_UNF_CIPHER_WORK_MODE_ECB;
		para.src = input_buf;
		para.dst = output_buf;
		para.datalen = AES_128_LEN;
		para.status = 0;
		ret = ioctl(fd_aes, IOCTL_AES_START_DE_PROCESSING, &para);
		if(ret < 0)
		{
			return ret;
		}
		memcpy(output + index, para.dst, AES_128_LEN);
	}
	last_pending = output[index - 1];

	int i = 0;
	for (i = 1; i < AES_128_LEN; i++)
	{
		if (output[index - 1 - i] != last_pending)
			break;
	}
	if ((int)last_pending == i)
	{
		index = index - i;
	}
	output[index]='\0';
	*output_len = index;
	return ret;
}
void crypto_aes128_init()
{
	if(isInited == false){
		fd_aes = open("/dev/aes",0);
		isInited = true;
	} 
}
void crypto_aes128_uninit()
{
	if(isInited == true)
	{
		close(fd_aes);
		isInited = false;
	}
}

int aes_pkcs5_padding(unsigned char *in, int in_len, unsigned char **out, int *out_len)
{
    if ((NULL == out) || (in == NULL) || (out_len == NULL))
    {
        return -1;
    }

    int mod = in_len % AES_128_LEN;
    int pad = AES_128_LEN - mod;

    *out_len = in_len + AES_128_LEN - mod;
    *out = (uint8_t*)malloc(*out_len * sizeof(uint8_t));
    if (*out == NULL)
    {
        return -1;
    }

    memcpy(*out, in, in_len);

    // add padding fields
    for (int i = in_len; i < *out_len; i++)
    {
        (*out)[i] = pad;
    }
    return 0;
}

int aes_pkcs5_unpadding(unsigned char *in, int in_len, unsigned char *out, int *out_len)
{
    if ((NULL == out) || (in == NULL) || (out_len == NULL))
    {
        return -1;
    }

    int remove_len = in[in_len - 1];
    if (remove_len > AES_128_LEN)
    {
        return -1;
    }
    *out_len = in_len - remove_len;
    memcpy(out, in, *out_len);
    return 0;
}

int mx_crypto_aes_cbc_encrypt_base64(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len)
{
	unsigned char *padded_input = NULL;
    int padded_input_len;
    mbedtls_aes_context ctx;
    uint8_t iv[AES_128_LEN];


	int ret = aes_pkcs5_padding((unsigned char*)input, input_len, &padded_input, &padded_input_len);
    if (ret != 0)
    {
		printf("aes_pkcs5_padding error\n");
        return -1;
    }

    // encrypt
    if (padded_input_len % AES_128_LEN)
    {
        return -1;
    }

    ret = mbedtls_aes_setkey_enc(&ctx, key, AES_128_LEN * 8);
    if (ret != 0)
    {
		printf("mbedtls_aes_setkey_enc error\n");
        return ret;
    }

    memcpy(iv, IV, AES_128_LEN);
    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, padded_input_len, iv, padded_input, output);
    if (ret != 0)
    {
		printf("mbedtls_aes_crypt_cbc error\n");
        return ret;
    }

    *output_len = padded_input_len;
	if(padded_input != NULL)
    	free(padded_input);
	mbedtls_aes_free(&ctx);
	return ret;
}

int mx_crypto_aes_cbc_encrypt(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len)
{

	if ((key == NULL) || (input == NULL) || (output == NULL))
    {
        return -1;
    }

	int enc_base64_len = 0;
	unsigned char* enc_base64_str = (unsigned char*)malloc(BUFFER_SIZE * 2);
	if (NULL == enc_base64_str)
	{
		return -1;
	}
	memset(enc_base64_str, 0, BUFFER_SIZE * 2); ///< TODO change长度

	int ret = mx_crypto_aes_cbc_encrypt_base64(key, input, input_len, enc_base64_str, &enc_base64_len);
	if(ret != 0)
	{
		printf("mx_crypto_aes_cbc_encrypt error\n");
		goto mx_crypto_aes_cbc_encrypt_error;
	}

	ret = mbedtls_base64_encode(output, BUFFER_SIZE * 2, (size_t *)output_len, (unsigned char *)enc_base64_str, enc_base64_len);
	if(ret != 0)
	{
		printf("mbedtls_base64_encode error\n");
		goto mx_crypto_aes_cbc_encrypt_error;
	}

mx_crypto_aes_cbc_encrypt_error:
	if(NULL != enc_base64_str)
	{
		free(enc_base64_str);
		enc_base64_str = NULL;
	}

    return ret;
}

int mx_crypto_aes_cbc_decrypt_base64(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len)
{
	unsigned char *padded_output = NULL;
    mbedtls_aes_context ctx;
    uint8_t iv[AES_128_LEN];
	int ret = 0;

    padded_output = (unsigned char*)malloc(input_len * sizeof(uint8_t));
    if (padded_output == NULL)
    {
		printf("padded_output malloc error \n");
		return -1;
    }

    mbedtls_aes_init(&ctx);

    // decrypt
    ret = mbedtls_aes_setkey_dec(&ctx, key, AES_128_LEN * 8);
    if (ret != 0)
    {
		printf("mbedtls_aes_setkey_dec error \n");
		return ret;
    }

    memcpy(iv, IV, AES_128_LEN);
    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, input_len, iv, input, padded_output);
    if (ret != 0)
    {
		printf("mbedtls_aes_crypt_cbc error \n");
		return ret;
    }

    ret = aes_pkcs5_unpadding(padded_output, input_len, output, output_len);
	if(ret != 0)
	{
		printf("aes_pkcs5_unpadding error \n");
		return ret;
	}
	if(padded_output != NULL)
    	free(padded_output);
	mbedtls_aes_free(&ctx);
	return ret;
}

int mx_crypto_aes_cbc_decrypt(const unsigned char *key,
        const unsigned char *input, int input_len,
        unsigned char *output, int *output_len)
{
    if ((key == NULL) || (input == NULL) || (output == NULL)) ///< 长度
    {
        return -1;
    }

	size_t dec_base64_len = 0;
	unsigned char* dec_base64_str = (unsigned char*)malloc(BUFFER_SIZE);
	if (NULL == dec_base64_str)
	{
		return -1;
	}
	memset(dec_base64_str, 0, BUFFER_SIZE);

	int ret = mbedtls_base64_decode((unsigned char *)dec_base64_str, BUFFER_SIZE, &dec_base64_len, (const unsigned char *)input, input_len);
	if(ret != 0)
	{
		printf("mbedtls_base64_decode error \n");
		goto mx_crypto_aes_cbc_decrypt_error;
	}

	ret = mx_crypto_aes_cbc_decrypt_base64(key, dec_base64_str, dec_base64_len, output, output_len);
	if(ret != 0)
	{
		printf("mx_crypto_aes_cbc_decrypt base64 error \n");
		goto mx_crypto_aes_cbc_decrypt_error;
	}

	output[(*output_len)] = '\0';

mx_crypto_aes_cbc_decrypt_error:
	if(NULL != dec_base64_str)
	{
		free(dec_base64_str);
		dec_base64_str = NULL;
	}
    return ret;
}

int crypto_sha256_calcu(const unsigned char *input, size_t len, 
	unsigned char *output)
{
	int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
	mbedtls_sha256_context ctx;

	mbedtls_sha256_init(&ctx);

	if ((ret = mbedtls_sha256_starts_ret(&ctx, 0)) != 0)
		goto crypto_sha256_calcu_error;

	if ((ret = mbedtls_sha256_update_ret(&ctx, input, len)) != 0)
		goto crypto_sha256_calcu_error;

	if ((ret = mbedtls_sha256_finish_ret(&ctx, output)) != 0)
		goto crypto_sha256_calcu_error;

crypto_sha256_calcu_error:
	mbedtls_sha256_free(&ctx);

	return ret;
}

int crypto_hmac_sha256(std::string strKey, 
	std::string strDID, std::string strMac, unsigned char *hmac)
{
	int ret;
	std::string strMsg(strDID);
	strMsg.append(strMac);
	strMsg.append(strKey);

	unsigned char acMsgSHA256[32] = { 0 };

	if ((ret = crypto_sha256_calcu((unsigned char*)strMsg.c_str(), 
		strMsg.length(), acMsgSHA256)) != 0)
		return ret;

	mbedtls_md_context_t ctx;
	const mbedtls_md_info_t *info;

	mbedtls_md_init(&ctx);
	info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

	ret = mbedtls_md_setup(&ctx, info, 1);
	if (ret != 0) 
	{
		goto crypto_hmac_sha256_error;
	}

	ret = mbedtls_md_hmac_starts(&ctx, (unsigned char *)strKey.c_str(),
		strKey.length());
	if (ret != 0) 
	{
		goto crypto_hmac_sha256_error;
	}

	ret = mbedtls_md_hmac_update(&ctx, acMsgSHA256, sizeof(acMsgSHA256));
	if (ret != 0) 
	{
		goto crypto_hmac_sha256_error;
	}

	ret = mbedtls_md_hmac_finish(&ctx, hmac);
	if (ret != 0) 
	{
		goto crypto_hmac_sha256_error;
	}

crypto_hmac_sha256_error:
	
	mbedtls_md_free(&ctx);

	return ret;
}

int crypto_aes256_encrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len)
{
	int ret = -1;
	int index = 0;
	unsigned char input_buf[AES_256_LEN] = { 0 };
	unsigned char output_buf[AES_256_LEN] = { 0 };

	mbedtls_aes_context aesContext;
	mbedtls_aes_init(&aesContext);

	ret = mbedtls_aes_setkey_enc(&aesContext, key, 256);
	if (ret != 0)
	{
		return ret;
	}

	for (index = 0; index <= len; index += AES_256_LEN)
	{
		memset(input_buf, 0, AES_256_LEN);
		memset(output_buf, 0, AES_256_LEN);

		if (index == len)
		{
			memset(input_buf, AES_256_LEN, AES_256_LEN);
		}
		else if (index + AES_256_LEN <= len)
		{
			memcpy(input_buf, input + index, AES_256_LEN);
		}
		else
		{
			memcpy(input_buf, input + index, len % AES_256_LEN);
			memset(input_buf + len % AES_256_LEN,
				AES_256_LEN - len % AES_256_LEN, 
				AES_256_LEN - len % AES_256_LEN);
		}

		ret = mbedtls_aes_crypt_ecb(&aesContext,
			MBEDTLS_AES_ENCRYPT, input_buf, output_buf);

		if (ret != 0)
		{
			return ret;
		}

		memcpy(output + index, output_buf, AES_256_LEN);
	}

	*output_len = index;
	return ret;
}

int crypto_aes256_encrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* base64_output_buffer,
	int base64_output_buffer_len,
	size_t *base64_output_len)
{
	unsigned char* output = (unsigned char*)malloc(len + 512);
	if (output == NULL)
		return -1;

	int output_len = 0;
	int ret = 0;
	ret = crypto_aes256_encrypt(key,
		input, len, output, &output_len);

	if (ret != 0)
	{
		free(output);
		return ret;
	}

	ret = mbedtls_base64_encode(base64_output_buffer,
		base64_output_buffer_len,
		base64_output_len,
		output, output_len);
	free(output);

	return ret;
}

int crypto_aes256_decrypt(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* out, int *output_len)
{
	int ret = -1;
	int index = 0;
	unsigned char output_buf[AES_256_LEN];
	char last_pending = '\0';

	if (input == NULL || out == NULL)
	{
		return -1;
	}
	mbedtls_aes_context aesContext;
	mbedtls_aes_init(&aesContext);

	ret = mbedtls_aes_setkey_dec(&aesContext, key, 256);
	if (ret != 0)
	{
		return ret;
	}

	for (index = 0; index < len; index += AES_256_LEN)
	{
		ret = mbedtls_aes_crypt_ecb(&aesContext,
			MBEDTLS_AES_DECRYPT, input + index,
			output_buf);
		if (ret != 0)
		{
			return ret;
		}

		memcpy(out + index, output_buf, AES_256_LEN * sizeof(char));
	}

	last_pending = out[index - 1];

	int i = 0;
	for (i = 1; i < AES_256_LEN; i++)
	{
		if (out[index - 1 - i] != last_pending)
			break;
	}
	if ((int)last_pending == i)
	{
		index = index - i;
	}

	out[index] = '\0';
	*output_len = index;
	return ret;
}

int crypto_aes256_decrypt_base64(unsigned char* key,
	unsigned char* input, int len,
	unsigned char* output, int *output_len)
{
	int ret = 0;
	if (key == NULL || input == NULL || output == NULL
		|| len <= 0)
		return -1;

	size_t decode_len = 0;

	if (strstr((const char *)input, "=="))
		decode_len = len / 4 * 3 - 2;
	else if (strstr((const char *)input, "="))
		decode_len = len / 4 * 3 - 1;
	else
		decode_len = len / 4 * 3;

	unsigned char *decode_input = (unsigned char *)malloc(decode_len);
	if (decode_input == NULL)
		return -1;

	size_t decode_out_len = 0;
	ret = mbedtls_base64_decode(decode_input,
		decode_len,
		&decode_out_len,
		input, len);

	if (ret != 0)
	{
		free(decode_input);
		return ret;
	}

	ret = crypto_aes256_decrypt(key,
		decode_input, decode_out_len,
		output, output_len);
	free(decode_input);
	return ret;
}

std::string crypto_sha256_base64(std::string strAuthInfo)
{
	int i = 0, ret = 0;
	unsigned char hash[32] = {0x00};
	mbedtls_sha256_context sha256_ctx;

	strAuthInfo += std::string(g_static_key);
	//printf("before SHA256 encrypt: %s\n", strAuthInfo.c_str());

    mbedtls_sha256_init(&sha256_ctx);
	mbedtls_sha256_starts(&sha256_ctx, 0); // 0表示传sha256 ， 1 表示传SHA-244
	mbedtls_sha256_update(&sha256_ctx, (unsigned char*)strAuthInfo.c_str(), strAuthInfo.length());
	mbedtls_sha256_finish(&sha256_ctx, hash);
	mbedtls_sha256_free(&sha256_ctx);

    printf("after SHA256 encrypt:");
	char strhash[64] = {0};
    for( i = 0; i < 32; i++)
    {
        printf("%02x", hash[i]);
		sprintf(strhash+i*2,"%02x", hash[i]);
    }
    printf("\r\n");

	unsigned char acAuthInfo[128] = { 0 };
	size_t iAuthInfoLen = 0;
	ret = mbedtls_base64_encode(acAuthInfo, sizeof(acAuthInfo),
		&iAuthInfoLen, (unsigned char*)strhash, 64);
	if (ret != 0)
		return "";

	return std::string((char*)acAuthInfo, iAuthInfoLen);
}

extern "C" {
	int sfc_boot(const char *u_boot_file_path);
}

void VerifyBoot()
{
	const char* lock_file_path = "/tmp/verifyboot.lock";
    int times = 0;
    int fd = open(lock_file_path,O_RDWR | O_CREAT);
	if(fd < 0) {
		exit(1);
	}

    while(0 != flock(fd,LOCK_EX | LOCK_NB ))
    {
		if(++times > 50) {
			flock(fd,LOCK_UN);
    		close(fd);
			exit(1);
		}
        usleep(200 * 1000);
    }

	// printf("%d:get lock\n",getpid());

    if(sfc_boot("/dev/mtd0") != 0) {
		flock(fd,LOCK_UN);
    	close(fd);
		exit(1);
	}
	// printf("%d:put lock\n",getpid());
    flock(fd,LOCK_UN);
    close(fd);
}

int verify_boot()
{
	static std::thread threadVerifyBoot(VerifyBoot);
	return 0;
}
