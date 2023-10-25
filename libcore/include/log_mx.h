#ifndef __LOG_MX_H__
#define __LOG_MX_H__
#include "type_def.h"
#include "global_export.h"
#include <string>

namespace maix {
	typedef enum
	{
		MX_LOG_ERROR,
		MX_LOG_WARN,
		MX_LOG_INFOR,
		MX_LOG_DEBUG,
		MX_LOG_TRACE,
	}E_MX_LOG_LEVEL;

	typedef enum
	{
		MX_LOG_NULL,
		MX_LOG_CONSOLE,
		MX_LOG_LOCAL,
		MX_LOG_REMOTE,
        MX_LOG_CONSOLE_AND_LOCAL
	}E_MX_LOG_TYPE;

	typedef enum
	{
		MX_LOG_TCP,
		MX_LOG_UDP,
	}E_MX_LOG_NET_TYPE;

	typedef struct _LogConfig
	{
		std::string			m_strName;
		E_MX_LOG_LEVEL		m_eLevel;
		E_MX_LOG_TYPE		m_eType;
		std::string			m_strFileName;
		E_MX_LOG_NET_TYPE	m_eNetType;
		std::string			m_strIP;
		int					m_iPort;
        std::string         m_strUnix;
	}T_LogConfig;

	MAIX_EXPORT std::string getLogTime(void);
	MAIX_EXPORT void logInit(T_LogConfig tLogConfig);
	MAIX_EXPORT void logPrint(E_MX_LOG_LEVEL eLevel, const char *format, ...);
    MAIX_EXPORT void mcu_logPrint(const char *format, ...);
}
#endif //__LOG_MX_H__
