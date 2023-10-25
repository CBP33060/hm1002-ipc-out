#ifndef __OTA_MANAGE_FW_CRC_H__
#define __OTA_MANAGE_FW_CRC_H__
namespace maix {
	unsigned int CRC32(const char* file);
	unsigned int CRC32(const char *file, int iOffset, size_t fileLen);
}
#endif //__OTA_MANAGE_FW_CRC_H__
