#include "efuse.h"
#include <common.h>
#include <irom.h>
#include <hash.h>
#include <rsa.h>
#include <scb_verify.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pss.h>
#include <cpm.h>
#include <sys/time.h>
#include <time.h>

extern unsigned int * cpm_vir_base;

int readEfuse(int seg_id, int offset, unsigned char *buf, int len);

static inline u32 get_rsa_e(void)
{
	return 0x10001;
}
/* 校验SPL中的rsa modulus n的值, 对rsa modulus做sha运算，对比efuse中存储的sha值
 * rsa_n_modulus rsa modulus的值的指针
 * */
static int verify_rsa_modulus(unsigned int *rsa_n_modulus)
{
	u32 rsa_n_mod_sha256[8],rsa_n_mod[8];
	int i,sc_en;

	hash_sha256((u32 *)rsa_n_modulus, SCBOOT_RSA_KEY_BYTE_LEN, rsa_n_mod_sha256);

	if(readEfuse(TRIM_DATA, 0, (unsigned char *)&sc_en, 1) != 0){
		return -1;
	}

	if((sc_en & 0x01) != 0) {
		if(readEfuse(SCB_DATA, 0, (unsigned char *)rsa_n_mod, 32)) {
			return -1;
		}
		for (i = 0; i < SCBOOT_RSA_N_SHA_WSIZE; i++) {
			// printf("0x%08x 0x%08x\n",rsa_n_mod_sha256[i],rsa_n_mod[i]);
			if (rsa_n_mod_sha256[i] != rsa_n_mod[i])
				return -1;
		}
	}
	
	return 0;
}

int cpm_init()
{
	int mmap_fd = open("/dev/mem", O_RDWR);
	cpm_vir_base = mmap(NULL, 0xe8, PROT_READ|PROT_WRITE, MAP_SHARED, mmap_fd, CPM_BASE);
	if(cpm_vir_base == NULL){
		close(mmap_fd);
		return -1;
	}
	close(mmap_fd);
	return 0;
}
int verify_spl(struct verify_info *v_info)
{
	unsigned int spl_sha_calculate[8];    //spl 计算得到sha256的值
	unsigned int spl_sha_restore[SCBOOT_RSA_KEY_WORD_LEN];      //spl 通过rsa还原得到的sha256值
	int i = 0,ret;

	if (v_info->spl_len == 0 || v_info->spl_len % 64 != 0) {
		goto err;
	}
	ret = cpm_init();
	if(ret == -1){
		goto err;
	}
	ret = hash_init();
	if(ret == -1){
		goto err;
	}	
	ret = rsa_init();
	if(ret == -1){
		goto err;
	}

	/* 1. 校验rsa modulus n的值 */
	if (verify_rsa_modulus(v_info->rsa_n_addr) != 0) {
		goto err;
	}
	/* 2. 将spl的签名还原为sha值 */
	do_rsa_2048(v_info->rsa_n_addr, get_rsa_e(),
				v_info->spl_sig_addr, spl_sha_restore, SCBOOT_RSA_KEY_WORD_LEN);

	/* 3. 计算spl的sha值 */
	hash_sha256(v_info->spl_addr, v_info->spl_len, spl_sha_calculate);
	for(i = 0; i < SCBOOT_SHA_WORD_LEN; i++) {
		spl_sha_calculate[i]=BLSWAP32(spl_sha_calculate[i]);
	}
	for (i = 0; i < SCBOOT_RSA_KEY_WORD_LEN; i++) {
		spl_sha_restore[i] = BLSWAP32(spl_sha_restore[i]);
	}
	struct Pss_Info_t info = {
		.mHash = (char *)spl_sha_calculate,
		.lenHash = SCBOOT_SHA_BYTE_LEN,
		.em = (char *)spl_sha_restore,
		.lenEm = SCBOOT_RSA_KEY_BYTE_LEN,
		.sha256 = sha256_bignum,
	};
	/*4. pss 校验*/
	if(pss_verify(&info) < 0) {
		goto err;
	}
	return 0;
err:
	return -1;
}
