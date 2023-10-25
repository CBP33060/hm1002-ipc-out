#ifndef COMMON_EXPORT_H
#define COMMON_EXPORT_H

#include "algorithm_sdk_error_code.h"
#include "mi_alg_type.h"
#include <vector>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if(defined WIN32 || defined_WIN32 || defined WINCE)
#define COMMON_DLL_EXPORT __declspec(dllexport)
#else
#define COMMON_DLL_EXPORT
#endif


namespace miot_ipc_algorithm {

#define ESC_START     "\033["
#define ESC_END       "\033[0m"
#define COLOR_FATAL   "31;40;5m"
#define COLOR_ALERT   "31;40;1m"
#define COLOR_CRIT    "31;40;1m"
#define COLOR_ERROR   "31;40;1m"
#define COLOR_WARN    "33;40;1m"
#define COLOR_NOTICE  "34;40;1m"
#define COLOR_INFO    "32;40;1m"
#define COLOR_DEBUG   "36;40;1m"
#define COLOR_TRACE   "37;40;1m"

// #define filename(x)   strrchr(x,'/') ? strrchr(x,'/')+1 : x

// #define LOG_INFO(format, args...) (printf( ESC_START COLOR_INFO "[I]-[%s]-[%s]-[%d]: \n" format ESC_END, filename(__FILE__), __FUNCTION__ , __LINE__, ##args))
// #define LOG_DEBUG(format, args...) (printf( ESC_START COLOR_DEBUG "[D]-[%s]-[%s]-[%d]:\n" format ESC_END, filename(__FILE__), __FUNCTION__ , __LINE__, ##args))
// #define LOG_WARN(format, args...) (printf( ESC_START COLOR_WARN "[W]-[%s]-[%s]-[%d]:\n" format ESC_END, filename(__FILE__), __FUNCTION__ , __LINE__, ##args))
// #define LOG_ERROR(format, args...) (printf( ESC_START COLOR_ERROR "[E]-[%s]-[%s]-[%d]:\n" format ESC_END, filename(__FILE__), __FUNCTION__ , __LINE__, ##args))

    typedef struct {
        int x;
        int y;
        int width;
        int height;
    } obj_box_t;

    typedef struct {
        obj_box_t obj_bbox;
        float obj_prob;
        int obj_class;
        std::string classname;
    } obj_info_t;

    struct Classify_Info {
        int obj_class;
        float obj_prob;
    };


#define MAX_DETECTOBJ_NUM 100

}
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


