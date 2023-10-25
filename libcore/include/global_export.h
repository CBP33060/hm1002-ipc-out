#ifndef __GLOBAL_EXPORT_H__
#define __GLOBAL_EXPORT_H__

#ifdef MAIX_BUILD_DLL
#define MAIX_EXPORT __declspec(dllexport)
#else
#define MAIX_EXPORT
#endif

#endif //__GLOBAL_EXPORT_H__
