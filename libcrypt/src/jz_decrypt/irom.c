#include <irom.h>
#include <common.h>
#include <cpm.h>
#include <scb_verify.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#define SPI_SIG_LEN	(4)
#define UBOOT_BUF_SIZE (48 * 1024)
u32 jump_offset = JUMP_OFFSET;
u32* heap_addr = 0;
typedef unsigned char uint8_t;

int sfc_boot(const char *u_boot_file_path)
{	
	heap_addr = malloc(HEAP_SIZE);
	uint8_t *uboot_buf = (uint8_t *)malloc(UBOOT_BUF_SIZE);
	memset(uboot_buf, 0, UBOOT_BUF_SIZE);

	int fd = open(u_boot_file_path,O_RDONLY);
    if(fd < 0) {
		free(uboot_buf);
        return -1;
    }
    int len = read(fd,uboot_buf,UBOOT_BUF_SIZE);
    if(len != UBOOT_BUF_SIZE) {
		free(uboot_buf);
        return -1;
    }
    close(fd);

	unsigned int spl_len = ((unsigned int*)uboot_buf)[3];
	int ret;
	struct verify_info v_info = {0};

	v_info.spl_sig_addr = (unsigned int *)(uboot_buf + SPL_SIG_SIZE);
	v_info.rsa_n_addr = (unsigned int *)(uboot_buf + SPL_SIG_SIZE + SCBOOT_RSA_KEY_BYTE_LEN);
	v_info.spl_addr = (unsigned int *)(uboot_buf + jump_offset);
	v_info.spl_len = spl_len - jump_offset;
	
	ret = verify_spl(&v_info);
	
	free(uboot_buf);
	free(heap_addr);
	return ret;
}
