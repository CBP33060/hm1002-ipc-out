#include <common.h>
#include <stdio.h>
#include <pss.h>

#define MAX_MD_SIZE 64

static int pss_mgf1(unsigned char *(*sha256)(const unsigned char *d, int n,
											  unsigned char *md), unsigned char *dst,
					 unsigned int dst_len, unsigned char *src, unsigned int src_len)
{

	if(!sha256)
		return -1;

	unsigned char tmp[MAX_MD_SIZE], tmp2[MAX_MD_SIZE];

	memset(tmp, 0, MAX_MD_SIZE);
	memset(tmp2, 0, MAX_MD_SIZE);
	unsigned char *p = dst, *ctr;
	unsigned int mask_len, hash_len = src_len, i;
	memcpy(tmp, src, src_len);
	ctr = tmp + src_len;

	while((int)dst_len > 0) {
		sha256(tmp, src_len + 4, tmp2);
		mask_len = dst_len < hash_len ? dst_len : hash_len;
		for(i = 0; i < mask_len; i++)
		{
			*p++ ^= tmp2[i];
		}
		dst_len -= mask_len;
		ctr[3]++;
	}

	return 0;
}

int pss_verify(struct Pss_Info_t *psInfo)
{
	if(!psInfo || !psInfo->sha256)
		return -1;

	unsigned int *mHash = (unsigned int *)(psInfo->mHash), selfmap[MAX_MD_SIZE];
	unsigned char tmp[8 + MAX_MD_SIZE * 2];
	memset(tmp, 0, 8 + MAX_MD_SIZE * 2);
	memset(selfmap, 0, MAX_MD_SIZE);
	unsigned char *map, *salt;
	unsigned int hLen = 32, MSBits;
	unsigned int maskedDBLen, i = 0;
	int nlen = 64;
	unsigned int emLen = 64 * 4;
	unsigned char *EM = (unsigned char *)psInfo->em;

	MSBits = (nlen * 32 - 1) & 0x7;

	if (EM[emLen - 1] != 0xbc) {
		return -1;
	}

	maskedDBLen = emLen - 1 - hLen;
	map = EM + maskedDBLen;

	if(pss_mgf1(psInfo->sha256, EM, maskedDBLen, map, hLen) < 0) {
		return -1;
	}

	if (MSBits)
		EM[0] &= 0xFF >> (8 - MSBits);

	for(i = 0;EM[i] == 0 && i<(maskedDBLen - 1); i++);

	if (EM[i++] != 0x01){
		return -1;
	}

	salt = EM + i;

	memcpy((tmp + 8), mHash, hLen);
	memcpy((tmp + 8 + hLen), salt, (maskedDBLen - i));
	psInfo->sha256((unsigned char *)tmp, 8 + hLen + (maskedDBLen -i), (unsigned char *)selfmap);

	if(memcmp(map, selfmap, 32)) {
		return -1;
	}
	return 0;
}
