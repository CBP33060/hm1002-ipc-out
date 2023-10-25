#ifndef _SCROM_RSA_H_
#define _SCROM_RSA_H_

//#define RSA_BASE        0xb34C0000
#define RSA_BASE        0x134C0000

#define RSAC            (0x0)
#define RSAE            (0x4)
#define RSAN            (0x8)
#define RSAM            (0xc)
#define RSAP            (0x10)

#define RSAC_RSA_INT_M  (1 << 17)
#define RSAC_PER_INT_M  (1 << 16)
#define RSAC_FIFO_EMPTY (1 << 9)
#define RSAC_RSA_2048   (1 << 7)
#define RSAC_RSAC       (1 << 6)
#define RSAC_RSAD       (1 << 5)
#define RSAC_RSAS       (1 << 4)
#define RSAC_PERC       (1 << 3)
#define RSAC_PERD       (1 << 2)
#define RSAC_PERS       (1 << 1)
#define RSAC_EN         (1 << 0)


int rsa_init();
int rsa_deinit();

int do_rsa_2048(unsigned int *n, unsigned int e,
				unsigned int *id, unsigned int *od,
				unsigned int klen);


#endif /* _SCROM_RSA_H_ */

