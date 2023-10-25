#ifndef __HASH_H__
#define __HASH_H__

//#define HASH_BASE		0xb3480000
#define HASH_BASE		0x13480000

#define HASH_REG_HSCR		0x0
#define HASH_REG_HSSR		0x4
#define HASH_REG_HSINTM		0x8
#define HASH_REG_HSSA		0xc
#define HASH_REG_HSTC		0x10
#define HASH_REG_HSDI		0x14
#define HASH_REG_HSDO		0x18
#define HASH_REG_HSCG       0x1c

#define HASH_DONE		(0x1 << 0)
#define HASH_DMADONE		(0x1 << 1)

#define HASH_SELECT_MD5		0x0
#define HASH_SELECT_SHA1	0x1
#define HSAH_SELECT_SHA256  0x3

#define HSCR_EN_SFT_BIT		0
#define HSCR_SEL_SFT_BIT	1
#define HSCR_INIT_SFT_BIT	4
#define HSCR_DMAE_SFT_BIT	5
#define HSCR_DMAS_SFT_BIT	6
#define HSCR_DIRVS_SFT_BIT	7
#define HSCR_DORVS_SFT_BIT	8


#define HASH_NEWROUND		(0x1 << 16)
#define HASH_ENDROUND		(0x1 << 18)
#define HASH_DMAMODE		(0x1 << 17)

#define HASH_BLOCK_BYTELEN	64
#define HASH_BLOCK_WORDLEN	16
#define HASH_PENDING_BYTELEN	8

int hash_init(void);
int hash_sha256(u32 *data, int size_block, u32 *data_out);

unsigned char *sha256_bignum(const unsigned char *d, int n,
						 unsigned char *md);

#endif
