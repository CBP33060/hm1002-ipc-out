#include "ota_fw_crc_pack.h"
#include "ota_fw_pack.h"

char g_buffer[G_BUFFER_LEN];
int g_file_num;

void enum_file_function(char *dir, char *pack)
{
	if(dir == NULL)
		return;

	char filename[1024]={ 0 };
	DIR *pdir = NULL;
	struct dirent* ent = NULL;

	pdir = opendir(dir);
	if(NULL == pdir)
	{
		return;
	}

	while(NULL != (ent=readdir(pdir)))
	{
		if((strcmp(ent->d_name, ".") == 0) ||
				(strcmp(ent->d_name, "..") == 0))
		{
			continue;
		}
		else
		{
			sprintf(filename, "%s/%s",dir,ent->d_name);
	
			if(ent->d_type == DT_DIR)
			{
				enum_file_function(filename, pack);
			}
			else
			{
				struct stat stat_buf;
				stat(filename, &stat_buf);

				struct stat pack_buf;
				stat(pack, &pack_buf);

				T_FILE_PROPERTY file_property;
				sprintf(file_property.file_name, "%s",ent->d_name);
				file_property.file_len = stat_buf.st_size;
				file_property.crc = CRC32(filename);
				file_property.offset = pack_buf.st_size + stat_buf.st_size + sizeof(T_FILE_PROPERTY);
				
				printf("%s len:%d offset:%d crc: %x\n", 
						file_property.file_name, 
						file_property.file_len,
						file_property.offset, 
						file_property.crc);

				FILE *fp = NULL;
				fp = fopen(pack,"ab");
				if(NULL == fp)
				{
					perror("open file failed");
					exit(-1);
				}
				fseek(fp,0,SEEK_END);
				fwrite(&file_property, sizeof(T_FILE_PROPERTY), 1, fp);
				
				if(stat_buf.st_size > 0)
				{
					char buf[FILE_READ_LEN]  = {0};
					FILE *fdr = NULL;
					fdr = fopen(filename, "rb");
					size_t file_len = stat_buf.st_size;

					while(file_len)
					{
						size_t read_len = 0;
						read_len = fread(buf, sizeof(char),FILE_READ_LEN, fdr);
						if(read_len > 0)
						{
							fwrite(buf,sizeof(char),read_len, fp);
							memset(buf, 0, FILE_READ_LEN);

							file_len = file_len - read_len;
						}

					}
					fclose(fdr);
				}
				fclose(fp);
				g_file_num ++;
			}
		}
	}

	closedir(pdir);
}
