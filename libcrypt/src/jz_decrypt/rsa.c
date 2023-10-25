#include <irom.h>
#include <common.h>
#include <cpm.h>
#include "rsa.h"
#include <bignum.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <cpm.h>
unsigned int * rsa_vir_base;
extern unsigned int * cpm_vir_base;
static void cpm_start_rsa()
{
	unsigned int * cpm_clkgr0 = (unsigned int *)((unsigned char *)cpm_vir_base + CPM_CLKGR0_OFFSET);
	// printf("%p\n",cpm_clkgr0);
	REG32(cpm_clkgr0) &= ~CLKGR0_RSA;
	// printf("%p\n",cpm_clkgr0);
}
int rsa_init()
{	
	cpm_start_rsa();
	int mmap_fd = open("/dev/mem", O_RDWR);
	rsa_vir_base = mmap(NULL, 776, PROT_READ|PROT_WRITE, MAP_SHARED, mmap_fd, RSA_BASE);
	if(rsa_vir_base == NULL){
		close(mmap_fd);
		return -1;
	}
	close(mmap_fd);
	return 0;
}
static void rsa_writel(unsigned int * virAddr,unsigned int val,int off)
{
	(*(volatile unsigned int *)((unsigned char*)virAddr + off) = val);
}
static unsigned int rsa_readl(unsigned int *virAddr,int off)
{
	return *(volatile unsigned int *)((unsigned char *)virAddr + off);
}
static void rsa_enable(void)
{
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) | RSAC_EN, RSAC);
}

static void rsa_disable(void)
{
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) & ~RSAC_EN, RSAC);
}

static void rsa_perpare_key(unsigned int *n, unsigned int e, unsigned int klen)
{
	unsigned int i;
	unsigned int tmp;

	/* sel rsa bit len */
	tmp = rsa_readl(rsa_vir_base, RSAC);

	if(klen == 32)
		tmp &= ~RSAC_RSA_2048;
	else
		tmp |= RSAC_RSA_2048;
	rsa_writel(rsa_vir_base, tmp, RSAC);

	/* set rsa E-key，先写全0，最后一个word写入e值 */
	for(i = 0; i < 63; i++)
		rsa_writel(rsa_vir_base, 0, RSAE);
	rsa_writel(rsa_vir_base, e, RSAE);

	/* set rsa N-key */
	for(i = 0; i < klen; i++)
		rsa_writel(rsa_vir_base, n[i], RSAN);

	/* set RSAC.PRES */
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) | RSAC_PERS, RSAC);

	/* polling perd, clear pers */
	while(!(rsa_readl(rsa_vir_base, RSAC) & RSAC_PERD));
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) & ~RSAC_PERS, RSAC);

	/* per clear */
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) | RSAC_PERC, RSAC);
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) & ~RSAC_PERC, RSAC);
}

static void rsa_do_crypt(unsigned int *id, unsigned int *od, unsigned int dlen)
{
	unsigned int i;

	/* set rsa MESSAGE */
	for(i = 0; i < dlen; i++)
		rsa_writel(rsa_vir_base, id[i], RSAM);

	/* set RSAC.RSAS */
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) | RSAC_RSAS, RSAC);

	/* polling rsad, clear rsas */
	while(!(rsa_readl(rsa_vir_base, RSAC) & RSAC_RSAD));
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) & ~RSAC_RSAS, RSAC);

	/* read output data,输出数据word大小端为反 */
	for(i = 0; i < dlen; i++) {
        while((rsa_readl(rsa_vir_base, RSAC) & RSAC_FIFO_EMPTY))
			;
		od[dlen - 1 - i] = rsa_readl(rsa_vir_base, RSAP);
	}
	/* rsa clear */
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) | RSAC_RSAC, RSAC);
	rsa_writel(rsa_vir_base, rsa_readl(rsa_vir_base, RSAC) & ~RSAC_RSAC, RSAC);
}

int do_rsa_2048(unsigned int *n, unsigned int e,
				unsigned int *id, unsigned int *od,
				unsigned int klen)
{
	rsa_enable();
	rsa_perpare_key(n, e, klen);
	rsa_do_crypt(id, od, klen);
	rsa_disable();

	return klen;
}
