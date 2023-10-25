#include <irom.h>
#include <common.h>
#include <cpm.h>
#include "hash.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <cpm.h>
unsigned int * hash_vir_base;
extern unsigned int * cpm_vir_base;

static void cpm_start_hash()
{
	unsigned int * cpm_clkgr0 = (unsigned int *)((unsigned char *)cpm_vir_base + CPM_CLKGR0_OFFSET);
	REG32(cpm_clkgr0) &= ~CLKGR0_SC_HASH;
}
int hash_init(void)
{
	cpm_start_hash();
	int mmap_fd = open("/dev/mem", O_RDWR);
	hash_vir_base = mmap(NULL, 32, PROT_READ|PROT_WRITE, MAP_SHARED, mmap_fd, HASH_BASE);
	if(hash_vir_base == NULL){
		close(mmap_fd);
		return -1;
	}
	close(mmap_fd);
	return 0;
}
static void hash_writel(unsigned int * virAddr,int off,unsigned int val)
{
	(*(volatile unsigned int *)((unsigned char*)virAddr + off) = val);
}
static unsigned int hash_readl(unsigned int *virAddr,int off)
{
	return *(volatile unsigned int *)((unsigned char *)virAddr + off);
}
int hash_sha256(u32 *data_in, int size, u32 *data_out)
{
	int i, j = 0, word_cnt = size / 4;
	u32 bit_len = BLSWAP32(size << 3);
	u32 size_block = size / 64;
	u32 padding = 0x00000080;
	u32 index = size & 0x3f;
	u32 padding_len;

	if (index < 56) {
		padding_len = 56 - index;
	} else {
		padding_len = 120 - index;
	}

	if (index == 0 || index >= 56)
		size_block++;


	hash_writel(hash_vir_base,HASH_REG_HSINTM, 0);
	hash_writel(hash_vir_base,HASH_REG_HSSR, 1);
	hash_writel(hash_vir_base,HASH_REG_HSCG, 0);
	hash_writel(hash_vir_base,HASH_REG_HSTC, size_block);
	hash_writel(hash_vir_base,HASH_REG_HSCR, 1 | 1 << 7 | 3 << 1);
	hash_writel(hash_vir_base,HASH_REG_HSCR, hash_readl(hash_vir_base,HASH_REG_HSCR) | 1 << 4);

	for(i = 0; i < word_cnt; i++) {
		hash_writel(hash_vir_base,HASH_REG_HSDI, data_in[i]);
		j++;
		if (j && j % 16 == 0) {
			while(!(hash_readl(hash_vir_base,HASH_REG_HSSR) & (1<<0)));
			hash_writel(hash_vir_base,HASH_REG_HSSR, 1);
		}
	}

	hash_writel(hash_vir_base,HASH_REG_HSDI, padding);
	j++;
	if (j % 16 == 0) {
		while(!(hash_readl(hash_vir_base,HASH_REG_HSSR) & (1<<0)));
		hash_writel(hash_vir_base,HASH_REG_HSSR, 1);
	}

	for(i = 0; i < padding_len / 4 - 1; i++) {
		hash_writel(hash_vir_base,HASH_REG_HSDI, 0);
		j++;
		if (j % 16 == 0) {
			while(!(hash_readl(hash_vir_base,HASH_REG_HSSR) & (1<<0)));
			hash_writel(hash_vir_base,HASH_REG_HSSR, 1);
		}
	}

	hash_writel(hash_vir_base,HASH_REG_HSDI, 0);
	hash_writel(hash_vir_base,HASH_REG_HSDI, bit_len);

	while(!(hash_readl(hash_vir_base,HASH_REG_HSSR) & (1<<0)));
	hash_writel(hash_vir_base,HASH_REG_HSSR, 1);

	for(i = 0; i < 8;i ++) {
		data_out[i] = hash_readl(hash_vir_base,HASH_REG_HSDO);
	}

    return 0;
}

unsigned char *sha256_bignum(const unsigned char *d, int n,
						 unsigned char *md)
{
	hash_sha256((unsigned int *)d, n, (unsigned int *)md);
	int i = 0;
	int *p = (int *)md;
	for(i = 0; i < SCBOOT_SHA_BYTE_LEN / sizeof(int); i++) {
		*(p + i) =	BLSWAP32(*(p + i));
	}
	return md;
}

#if 0
extern int cpm_init();
extern int rsa_init();
int main(int argc, char **argv)
{
	cpm_init();
	hash_init();
	rsa_init();

	int _fd = open(argv[1], O_RDONLY);
	int size = lseek(_fd,0,SEEK_END);
	printf("size:%d\n",size);
	int plainlen = size;
	close(_fd);
	_fd = open(argv[1], O_RDONLY);
	
	// static unsigned char file_buf[3694249];

	unsigned char *file_buf = (unsigned char *)malloc(plainlen);
	if(file_buf == NULL) 
	{
		printf("file_buf is NULL\n");
		return -1;
	}
	size = read(_fd, file_buf, plainlen);
	printf("read size:%d\n",size);
	close(_fd);

	unsigned char md[1024];

	sha256_bignum(file_buf, plainlen, md);

	printf("%s\n",md);

	return 0;
}
#endif