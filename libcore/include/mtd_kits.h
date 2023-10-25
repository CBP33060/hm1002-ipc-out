#ifndef MTD_KITS_H
#define MTD_KITS_H

#include <sys/ioctl.h>
#include <sys/mount.h>
#include <mtd/mtd-user.h>
#include <mutex> 

typedef unsigned int uint;
typedef unsigned short uint16;
typedef long long loff_t;

struct mtd_info {
    int fd;
    uint mtd_size;
    uint blk_cnt;
    uint blk_size;
    uint16* blk_map;
    unsigned char *blk_cache;
    loff_t offset;
};

extern int mtd_write(mtd_info *mtdInfo, char* buf, size_t len);
extern int mtd_read(mtd_info *mtdInfo, char* buf, size_t len);
extern loff_t mtd_lseek(mtd_info *mtdInfo, loff_t offset, int whence);
extern int mtd_open(mtd_info *mtdInfo, const char* name, int oflag);
extern int mtd_close(mtd_info *mtdInfo);

#endif
