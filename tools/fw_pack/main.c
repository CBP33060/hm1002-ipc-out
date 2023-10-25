#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "ota_fw_crc_pack.h"
#include "ota_fw_pack.h"

extern int g_file_num;

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		printf("usage: ota_fw_pack  [dir] [package]");
		exit(-1);
	}

	T_FILE_PACK_HEADER header;
	sprintf(header.file_flag, "%s", "IPC_OUT_BIN");
	header.file_num = 0;
	header.offset = sizeof(T_FILE_PACK_HEADER);

	FILE *fp = NULL;
	fp = fopen(argv[2],"wb+");
	if(NULL == fp)
	{
		perror("open file failed");
	}

	fwrite(&header, sizeof(T_FILE_PACK_HEADER), 1, fp);
	fclose(fp);
	enum_file_function(argv[1], argv[2]);

	header.file_num = g_file_num;

	fp = fopen(argv[2],"rb+");
	if(NULL == fp)
	{
		perror("open file failed");
	}
	fseek(fp,0,SEEK_SET);

	fwrite(&header, sizeof(T_FILE_PACK_HEADER), 1, fp);
	fclose(fp);
	
	return 0;
}
