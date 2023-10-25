#ifndef __SCB_VERIFY_H__
#define __SCB_VERIFY_H__

#include <common.h>
#include <irom.h>

struct verify_info
{
	unsigned int *spl_sig_addr;			/*< spl 签名的地址，数据内容长度为256 byte >*/
	unsigned int *rsa_n_addr;			/*< rsa modulus n的地址， 数据内容长度256 byte >*/
	unsigned int *spl_addr;			/*< spl 的地址 >*/
	int spl_len;						/*< spl实际长度, 单位byte >*/
};
int verify_spl(struct verify_info *v_info);

#endif
