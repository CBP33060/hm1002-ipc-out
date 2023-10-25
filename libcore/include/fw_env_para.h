#ifndef __FW_ENV_PARA_H__
#define __FW_ENV_PARA_H__
#include <string>
#include <mutex>
#include <future>
#include "type_def.h"
#include "log_mx.h"
#include <type_traits>
namespace maix {
	char * getFWParaConfig(const char * key);
	char * getFWParaConfig(const char * section_name,const char * key);
	int setFWParaConfig(const char *name, const char *value, int overwrite);
	int setFWParaConfig(const char * section_name,const char *key, const char *value, int overwrite);
	int saveFWParaConfig(void);
	int unsetFWParaConfig(const char *name);
	int unsetFWParaConfig(const char * section_name,const char *name);
	int clearFWParaConfig(void);
}
#endif //__APP_H__