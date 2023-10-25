#ifndef __XMAI_4G_PACK_H__
#define __XMAI_4G_PACK_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define FILE_FLAG_LEN 256   
#define FILE_NAME_LEN 256
#define G_BUFFER_LEN 100
#define FILE_READ_LEN 128

typedef struct _file_pack_header
{
	char file_flag[FILE_FLAG_LEN];
	int file_num;
	int offset;
}T_FILE_PACK_HEADER;

typedef struct _file_property
{
	char file_name[FILE_NAME_LEN];
	unsigned int file_len;
	unsigned int crc;
	int offset;
}T_FILE_PROPERTY;

void enum_file_function(char *dir, char *pack);
#endif
