#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include "mtd_kits.h"


#define BLK_INVALID ((uint16)-1)

static int get_mtd_info(int fd, mtd_info_t* mtd_info)
{
    int ret = 0;

    if (fd < 0 || mtd_info == NULL) {
        return ret;
    }

    if (!ioctl(fd, MEMGETINFO, mtd_info)) {
        ret = 1;
    }

    return ret;
}

static int is_bad_block(mtd_info *mtdInfo, loff_t offset)
{
    int ret = 0;

    if (mtdInfo->fd < 0) {
        return ret;
    }

    if (ioctl(mtdInfo->fd, MEMGETBADBLOCK, &offset) > 0) {
        printf("Block at offset 0x%x is bad\n", (int)offset);
        ret = 1;
    }

    return ret;
}

static int MTDRawErase(mtd_info *mtdInfo, uint16 si)
{
    if (si < mtdInfo->blk_cnt) {
        int blk_index = mtdInfo->blk_map[si];
        erase_info_t erase;

        erase.start = blk_index * mtdInfo->blk_size;
        erase.length = mtdInfo->blk_size;
        if (ioctl(mtdInfo->fd, MEMERASE, &erase)) {
            printf("Error: MEMERASE failed\n");
            return 0;
        } else {
            return 1;
        }
    } else {
        printf("[%s] Blk index (%d) error! Blk %d.\n",__FUNCTION__, (int)si, (int)mtdInfo->blk_cnt);
        return 0;
    }
}

static int MTDMarkBadBlock(mtd_info *mtdInfo, uint16 si)
{
    if (si < mtdInfo->blk_cnt) {
        int blk_index = mtdInfo->blk_map[si];
        loff_t blk_addr = blk_index * mtdInfo->blk_size;

        if (ioctl(mtdInfo->fd, MEMSETBADBLOCK, &blk_addr)) {
            printf("Error: MEMSETBADBLOCK failed\n");
            return 0;
        } else {
            return 1;
        }
    } else {
        printf("[%s] Blk index (%d) error! Blk %d.\n",__FUNCTION__, (int)si, (int)mtdInfo->blk_cnt);
        return 0;
    }
}

static int MTDRawWrite(mtd_info *mtdInfo, uint16 blkIdx, uint blkOff, char* buf, size_t len)
{
    int ret = -1;
    uint16 blkIdx_tmp = blkIdx;
    int cnt = 4;

BAD_BLOCK:
    if(blkIdx < mtdInfo->blk_cnt) {
        blkIdx = mtdInfo->blk_map[blkIdx];

        memset(mtdInfo->blk_cache, 0xFF, mtdInfo->blk_size);
        if ((blkOff != 0) || (blkOff + len != mtdInfo->blk_size)) {    
            lseek(mtdInfo->fd, blkIdx * mtdInfo->blk_size, SEEK_SET);
            read(mtdInfo->fd, mtdInfo->blk_cache, mtdInfo->blk_size);
            memcpy(mtdInfo->blk_cache + blkOff, buf, len);
        } else {
            memcpy(mtdInfo->blk_cache, buf, len);
        }

        while(cnt--) {
            if (MTDRawErase(mtdInfo, blkIdx_tmp) == 0) {
                 printf("[%s] erase flash error!\n",__FUNCTION__);
                 continue;
            }
    
            lseek(mtdInfo->fd, blkIdx * mtdInfo->blk_size, SEEK_SET);
            if ( (ret = write(mtdInfo->fd, mtdInfo->blk_cache, mtdInfo->blk_size)) <= 0 ) {
                 printf("[%s]write error\n",__FUNCTION__);
                 continue;
            } else {
                break;
            }
        }

        if (cnt == 0) {
            printf("[%s]may be bad block\n", __FUNCTION__);
            if (MTDMarkBadBlock(mtdInfo, blkIdx_tmp) == 0) {
                printf("[%s]mark bad block %d error!\n",__FUNCTION__, blkIdx_tmp);
                return -1;
            } else {
                printf("[%s]find next good block\n",__FUNCTION__);
                memmove(&mtdInfo->blk_map[blkIdx_tmp], &mtdInfo->blk_map[blkIdx_tmp + 1], (mtdInfo->blk_cnt - blkIdx_tmp - 1) * sizeof(uint16));
                mtdInfo->blk_cnt--;
                mtdInfo->blk_map[mtdInfo->blk_cnt] = BLK_INVALID;
                blkIdx = blkIdx_tmp;
                goto BAD_BLOCK;
            }
        } else {
            return len;
        }
    }else {
        return -1;
    }
}

static int MTDRawRead(mtd_info *mtdInfo, uint16 blkIdx, uint blkOff, char* buf, size_t len)
{
    int ret = -1;
    if(blkIdx < mtdInfo->blk_cnt) {
        blkIdx = mtdInfo->blk_map[blkIdx];
        lseek(mtdInfo->fd, blkIdx * mtdInfo->blk_size + blkOff, SEEK_SET);

        if ( (ret = read(mtdInfo->fd, buf, len)) < 0 ) {
            printf("read error");
        }
    }

    return ret;
}

int mtd_write(mtd_info *mtdInfo, char* buf, size_t len)
{
    int ret = -1;
    uint16 blkIdx = 0;
    uint blkOff = 0;
    uint wlen;
    int sumLen = 0;
    int tmpLen = 0;

    if (mtdInfo->fd < 0 || len <= 0 || buf == NULL) {
        printf("mtd_write error\n");
        return ret;
    }

    while (len) {
        blkIdx = mtdInfo->offset / mtdInfo->blk_size;
        blkOff = mtdInfo->offset % mtdInfo->blk_size;
        wlen = (mtdInfo->blk_size - blkOff) < len ? (mtdInfo->blk_size - blkOff) : len;

        // printf("write gOffset = 0x%llx, wlen = %x, blkIdx = %x, blkOff = %x, len_left:%d\n", mtdInfo->offset, wlen, blkIdx, blkOff, len);
        if ( (tmpLen = MTDRawWrite(mtdInfo, blkIdx, blkOff, buf, wlen)) <= 0 ) {
            printf("MTDRawWrite error\n");
            break;
        }

        sumLen += tmpLen;
        len -= wlen;
        mtdInfo->offset += wlen;  
        buf += wlen;
    }

    return sumLen == 0 ? -1 : sumLen;
}

int mtd_read(mtd_info *mtdInfo, char* buf, size_t len)
{
    int ret = -1;
    uint16 blkIdx = 0;
    uint blkOff = 0;
    uint wlen;
    size_t actual_wlen = 0;

    if (mtdInfo->fd < 0 || len <= 0 || buf == NULL) {
        printf("mtd_read error\n");
        return ret;
    }

    while (len) {
        blkIdx = mtdInfo->offset / mtdInfo->blk_size;
        blkOff = mtdInfo->offset % mtdInfo->blk_size;
        wlen = (mtdInfo->blk_size - blkOff) < len ? (mtdInfo->blk_size - blkOff) : len;

        if ( (ret = MTDRawRead(mtdInfo, blkIdx, blkOff, buf, wlen)) <= 0 ) {
            printf("MTDRawRead error\n");
            break;
        }

        // printf("read gOffset = 0x%llx, wlen = %d, blkIdx = %d, blkOff = %d, ret:%d:len:%d\n", mtdInfo->offset, wlen, blkIdx,  blkOff, ret, len);
        actual_wlen += ret;
        len -= wlen;
        mtdInfo->offset += wlen;
        buf += wlen;
    }

    return actual_wlen;
}

loff_t mtd_lseek(mtd_info *mtdInfo, loff_t offset, int whence)
{
    switch (whence) {
        case SEEK_SET:
            mtdInfo->offset = offset;
            break;
        case SEEK_CUR:
            mtdInfo->offset += offset;
            break;
        case SEEK_END:
            mtdInfo->offset = mtdInfo->blk_size * mtdInfo->blk_cnt + offset;
            break;
        default:
            break;
    }

    return mtdInfo->offset;
}

int mtd_open(mtd_info *mtdInfo, const char* name, int oflag)
{
    int fd = -1;
    mtd_info_t info;
    uint16 i = 0;
    loff_t bpos = 0;

    if ( (fd = open(name, oflag)) < 0) {
        printf("mtd_open error\n");
        return -1;
    }

    printf("mtd_open fd: %d\n", fd);

    if (get_mtd_info(fd, &info) == 0) {
        printf("[%s]get mtd info error!\n",__FUNCTION__);
        close(fd);
        return -1;
    }

    printf("info.size=%d\n", info.size);
    printf("info.erasesize=%d\n", info.erasesize);
    printf("info.writesize=%d\n", info.writesize);
    printf("info.oobsize=%d\n", info.oobsize);
    mtdInfo->fd = fd;
    mtdInfo->mtd_size = info.size;
    mtdInfo->blk_cnt = info.size / info.erasesize;
    mtdInfo->blk_size =  info.erasesize;
    mtdInfo->blk_map = (uint16*)malloc(mtdInfo->blk_cnt * sizeof(uint16));
    mtdInfo->blk_cache = (unsigned char*)malloc(mtdInfo->blk_size);
    if (mtdInfo->blk_map == NULL || mtdInfo->blk_cache == NULL) {
        printf("[%s]mem error!\n",__FUNCTION__);
        mtd_close(mtdInfo);
        return -1;
    }
    memset(mtdInfo->blk_map, BLK_INVALID, mtdInfo->blk_cnt * sizeof(uint16));
    mtdInfo->offset = 0;
    for (bpos = 0; (bpos < mtdInfo->mtd_size) && (i < mtdInfo->blk_cnt); bpos += mtdInfo->blk_size) {
        if (is_bad_block(mtdInfo, bpos)) {
            printf("[0x%llx] is bad block\n", bpos);
            continue;
        } else {
            mtdInfo->blk_map[i++] = bpos / mtdInfo->blk_size;
        }
    }
    mtdInfo->blk_cnt = i;
    printf("gBlks = 0x%x\n", mtdInfo->blk_cnt);

    return fd;
}


int mtd_close(mtd_info *mtdInfo)
{
    printf("mtd_close fd: %d\n", mtdInfo->fd);
    if (mtdInfo->fd >= 0) {
        if (mtdInfo->blk_map) {
            free(mtdInfo->blk_map);
        }
        if (mtdInfo->blk_cache) {
           free(mtdInfo->blk_cache);
        }
        mtdInfo->mtd_size = 0;
        mtdInfo->blk_cnt = 0;
        mtdInfo->blk_size = 0;
        mtdInfo->blk_map = NULL;
        mtdInfo->blk_cache = NULL;
        mtdInfo->offset = 0;
        close(mtdInfo->fd);
    }
    return 0;
}

