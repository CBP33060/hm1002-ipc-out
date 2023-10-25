#ifndef __OTA_MANAGE_FW_UNPACK_H__
#define __OTA_MANAGE_FW_UNPACK_H__
#include "type_def.h"
#include "log_mx.h"
#include "mtd_kits.h"
#define FILE_FLAG_LEN 256   
#define	FILE_NAME_LEN 256
#define G_BUFFER_LEN 100
#define FILE_READ_LEN 128*1024

namespace maix {
	typedef struct _file_pack_header
	{
		char file_flag[FILE_FLAG_LEN];
		int file_num;
		int offset;
	} T_FILE_PACK_HEADER;

	typedef struct _file_property
	{
		char file_name[FILE_NAME_LEN];
		unsigned int file_len;
		unsigned int crc;
		int offset;
	} T_FILE_PROPERTY;

	// typedef enum __firmware_type
	// {
	// 	E_TYPE_FILE = 0,
	// 	E_TYPE_MTD = 1
	// } E_FIRMWARE_TYPE;

	// typedef union _firmware_fd_union
	// {
	// 	int m_fd;
	// 	mtd_info m_info;
	// } U_FIRMWARE_FD;

	// typedef struct _firmware_property
	// {
	// 	U_FIRMWARE_FD m_Fd;
	// 	E_FIRMWARE_TYPE m_Type;
	// } T_FIRMWARE_PROPERTY;

	mxbool unpackFirmwareBin(const char* file, const char *srcName, const char *dstName, int headerOffset, bool isMtd);
	mxbool backupFirmwareBin(const char *srcName, const char *dstName);
}
#endif //__OTA_MANAGE_FW_UNPACK_H__