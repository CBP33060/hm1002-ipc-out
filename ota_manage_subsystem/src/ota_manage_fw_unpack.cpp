#include "ota_manage_fw_unpack.h"
#include "log_mx.h"
#include "ota_manage_fw_crc.h"
#include <string.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "fw_env_para.h"
#include "common.h"
#include <sys/types.h>
#include <unistd.h>
#include "mtd_kits.h"
 
namespace maix {
    int ota_write(int fd, void* buf, size_t len, mtd_info* mtdInfo)
    {
		int ret = 0;
        if (NULL != mtdInfo)
        {
            ret = mtd_write(mtdInfo, (char *)buf, len);
        }
        else
        {
            ret = write(fd, buf, len);
        }
		return ret;
    }

    int ota_read(int fd, void* buf, size_t len, mtd_info* mtdInfo)
    {
		int ret = 0;
        if (NULL != mtdInfo)
        {
            ret = mtd_read(mtdInfo, (char *)buf, len);
        }
        else
        {
            ret = read(fd, buf, len);
        }
		return ret;
    }

    loff_t ota_lseek(int fd, loff_t offset, int whence, mtd_info* mtdInfo)
    {
		loff_t ret = 0;
        if (NULL != mtdInfo)
        {
            ret = mtd_lseek(mtdInfo, offset, whence);
        }
        else
        {
            ret = lseek(fd, offset, whence);
        }
		return ret;
    }

    int ota_open(const char* name, int oflag, mtd_info* mtdInfo)
    {
		int ret = 0;
        if (NULL != mtdInfo)
        {
           ret = mtd_open(mtdInfo, name, oflag);
        }
        else
        {
           ret = open(name, oflag);
        }

		return ret;
    }

    int ota_close(int fd, mtd_info* mtdInfo)
    {
		int ret = 0;
        if (NULL != mtdInfo)
        {
        	ret = mtd_close(mtdInfo);
        }
        else
        {
            ret = close(fd);
        }
		return ret;
    }

	mxbool safe_erase(const char *path)
	{
		std::string strValue;

		linuxPopenExecCmd(strValue, "flash_eraseall %s", path);
		logPrint(MX_LOG_DEBUG, "flash erase all:%s", path);
		return mxtrue;
	}

	/// kernel和rootfs有四个字节的头部信息
	mxbool unpackFirmwareBin(const char* file, const char *srcName, const char *dstName, int headerOffset, bool isMtd)
	{
		if ((NULL == file) || (NULL == srcName) || (NULL == dstName))
		{
			logPrint(MX_LOG_ERROR, "uppack firmware file is null");
			return mxfalse;
		}
		mxbool bFind = mxfalse;
		T_FILE_PACK_HEADER header = {0};
		mtd_info *mtdInfo = NULL;

		int fp = ota_open(file, O_RDONLY, NULL);
		if (fp < 0)
		{
			logPrint(MX_LOG_ERROR, "ota_open file failed");
			return mxfalse;
		}
		ota_lseek(fp, 0, SEEK_SET, NULL);

		ota_read(fp, &header, sizeof(T_FILE_PACK_HEADER), NULL);
		logPrint(MX_LOG_DEBUG, "%s %d %d\n", 
			header.file_flag, header.file_num, header.offset);

		int i = 0;
		size_t offset = header.offset;
		for (i = 0; i < header.file_num; i++)
		{
			ota_lseek(fp, offset, SEEK_SET, NULL);

			T_FILE_PROPERTY file_property;
			memset(&file_property, 0, sizeof(T_FILE_PROPERTY));
			ota_read(fp, &file_property, sizeof(T_FILE_PROPERTY), NULL);
			logPrint(MX_LOG_DEBUG,"file_name: %s %s %d %d %x\n",
				file_property.file_name, srcName, file_property.file_len, 
				file_property.offset, file_property.crc);
			/// todo: src 不足dst的保护，做一个保护机制
			if ((0 == strncmp(srcName, file_property.file_name, strlen(srcName))
														&& (file_property.file_len > 0)))
			{
				/// ota_close file and check crc32
				long tmpOffset = ota_lseek(fp, 0, SEEK_CUR, NULL);
				ota_close(fp, NULL);

				unsigned int crc = CRC32(file, file_property.offset - file_property.file_len, file_property.file_len);
				if (crc != file_property.crc)
				{
					logPrint(MX_LOG_ERROR, "calculate crc:%x, file crc:%x, is not same\n", crc, file_property.crc);
					goto unpack_exit;
				}
				logPrint(MX_LOG_DEBUG, "crc is same, %x\n", crc);

				fp = ota_open(file, O_RDONLY, NULL);
				ota_lseek(fp, tmpOffset + headerOffset, SEEK_SET, NULL);

				if (isMtd)
				{
					safe_erase(dstName);
					mtdInfo = (mtd_info *)malloc(sizeof(mtd_info));
					if(NULL == mtdInfo)
					{
						logPrint(MX_LOG_ERROR, "mtd info malloc failed");
						goto unpack_exit;
					}
					memset(mtdInfo, 0, sizeof(mtd_info));
				}

				int fpw = ota_open(dstName, O_SYNC | O_RDWR | O_CREAT, mtdInfo);
				if (fpw < 0)
				{
					logPrint(MX_LOG_ERROR, "fpw ota_open failed");
					goto unpack_exit;
				}
				ota_lseek(fpw, 0, SEEK_SET, mtdInfo);

				char buf[FILE_READ_LEN] = { 0 };
				size_t file_len = file_property.file_len - headerOffset;
				size_t readNum = 0;
				while (file_len > 0)
				{
					readNum = (file_len > FILE_READ_LEN) ? FILE_READ_LEN : file_len;
					size_t readLen = 0;
					readLen = ota_read(fp, buf, readNum, NULL);
					if (readLen != readNum)
					{
						logPrint(MX_LOG_ERROR, "ota_read failed, readLen %d, readNum:%d", readLen, readNum);
						ota_close(fpw, mtdInfo);
						ota_close(fp, NULL);
						goto unpack_exit;
					}
					size_t writeLen = 0;
					writeLen = ota_write(fpw, buf, readLen, mtdInfo);
					if (writeLen != readLen)
					{
						logPrint(MX_LOG_ERROR, "ota_write failed, readLen %d, writeLen:%d", readLen, writeLen);
						ota_close(fpw, mtdInfo);
						ota_close(fp, NULL);
						goto unpack_exit;
					}
					memset(buf, 0, FILE_READ_LEN);

					file_len = file_len - readLen;
				}
				ota_close(fpw, mtdInfo);
				bFind = mxtrue;
				break;
			}
			offset = file_property.offset;
		}
		ota_close(fp, NULL);

unpack_exit:
		if (mtdInfo)
		{
			free(mtdInfo);
			mtdInfo = NULL;
		}
		if (mxfalse == bFind)
		{
			logPrint(MX_LOG_ERROR, "unpack firmware bin not complete :%s", srcName);
			return mxfalse;
		}

		return mxtrue;
	}


	mxbool backupFirmwareBin(const char *srcName, const char *dstName)
	{
		if ((NULL == srcName) || (NULL == dstName))
		{
			logPrint(MX_LOG_ERROR, "backup firmware file is null");
			return mxfalse;
		}
		safe_erase(dstName);

		mtd_info mtdSrcInfo = {0};
		memset(&mtdSrcInfo, 0, sizeof(mtdSrcInfo));
		mtd_info mtdDstInfo = {0};
		memset(&mtdDstInfo, 0, sizeof(mtdDstInfo));

		int fp = ota_open(srcName, O_RDONLY, &mtdSrcInfo);
		int fpw = ota_open(dstName, O_SYNC | O_RDWR | O_CREAT, &mtdDstInfo);
		if ((fp < 0) || (fpw < 0))
		{
			logPrint(MX_LOG_ERROR, "ota_open file failed");
			return mxfalse;
		}	
		ota_lseek(fpw, 0, SEEK_SET, &mtdDstInfo);
		ota_lseek(fp, 0, SEEK_SET, &mtdSrcInfo);	

		unsigned int fileLen = ota_lseek(fp, 0, SEEK_END, &mtdSrcInfo) + 4;
		unsigned int writeFileLen = htonl(fileLen);
		fileLen -= 4;

		ota_lseek(fp, 0, SEEK_SET, &mtdSrcInfo);

		logPrint(MX_LOG_DEBUG, "ota_write file len:%u, header len:%d, sizeof:%d", fileLen, sizeof(fileLen)/sizeof(char), sizeof(unsigned int));

		long offset = 0;
		int readLen = 0;
		int writeLen = 0;
		int readNum = 0;
		unsigned char buf[FILE_READ_LEN] = { 0 };
		int iFirstOff = 4; ///< 从nor flash 备份数据到 nand flash 时，需要添加四个字节的头部信息
		ota_write(fpw, &writeFileLen, iFirstOff, &mtdDstInfo);
		while(fileLen > 0)
		{
			readNum = (fileLen > FILE_READ_LEN) ? FILE_READ_LEN : fileLen;
			// readNum -= iFirstOff;
			readLen = ota_read(fp, buf, readNum, &mtdSrcInfo);
			if (readLen != readNum)
			{
				logPrint(MX_LOG_ERROR, "ota_read failed, readLen %d, readNum:%d", readLen, readNum);
				ota_close(fpw, &mtdDstInfo);
				ota_close(fp, &mtdSrcInfo);
				return mxfalse;
			}
			
			writeLen = ota_write(fpw, buf, readLen, &mtdDstInfo);
			if (writeLen != readLen)
			{
				ota_close(fpw, &mtdDstInfo);
				ota_close(fp, &mtdSrcInfo);
				logPrint(MX_LOG_ERROR, "backup firmware failed, readLen = %d, writeLen = %d", readLen, writeLen);
				return mxfalse;
			}

			offset += writeLen;
			fileLen -= writeLen;
			memset(buf, 0, sizeof(buf));
			// logPrint(MX_LOG_DEBUG, "backup firmware readLen len = %d, all offset:%d, file len:%d", readLen, offset, fileLen);
		}

		ota_close(fpw, &mtdDstInfo);
		ota_close(fp, &mtdSrcInfo);

		logPrint(MX_LOG_DEBUG, "backup firmware success, src:%s to dst:%s", srcName, dstName);
		return mxtrue;
	}
}