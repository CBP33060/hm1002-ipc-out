#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include "fw_env_para.h"
namespace maix {

    #define ENV_IOC_MAGIC    'e'
    #define ENV_IOC_MAXNR    5
    #define ENV_IOCGET        _IOR(ENV_IOC_MAGIC, 0, unsigned long)
    #define ENV_IOCSET        _IOW(ENV_IOC_MAGIC, 1, unsigned long)
    #define ENV_IOCUNSET    _IOW(ENV_IOC_MAGIC, 2, unsigned long)
    #define ENV_IOCCLR        _IOW(ENV_IOC_MAGIC, 3, unsigned long)
    #define ENV_IOCPRT        _IOR(ENV_IOC_MAGIC, 4, unsigned long)
    #define ENV_IOCSAVE        _IOR(ENV_IOC_MAGIC, 5, unsigned long)

    #define ENV_NAME_MAXLEN    64

    #ifdef CONFIG_ENV_MAXLEN
    #define ENV_VALUE_MAXLEN    CONFIG_ENV_MAXLEN
    #else
    #define ENV_VALUE_MAXLEN    0x1000
    #endif
    typedef struct env_ioctl_args
    {
        char section_name[ENV_NAME_MAXLEN];
        char key[ENV_NAME_MAXLEN];
        char * buf;
        int maxlen;
        int  overwrite;
    }env_ioctl_args_t;

    static char env_value_buf[ENV_VALUE_MAXLEN];
    static int env_value_off = 0;

    static pthread_mutex_t env_value_mutex = PTHREAD_MUTEX_INITIALIZER;
    static int dev_fd = -1;

    static int env_open(void)
    {
        const char * devname = "/dev/env";
        if (dev_fd < 0) {
            dev_fd = open(devname, O_RDWR);
            if (dev_fd < 0) {
                fprintf(stderr, "ERROR: Cannot open %s,err:%d\n", devname,dev_fd);
                return -1;
            }
        }
        return 0;
    }

    #define CHECK_ENV_OPEN()    \
        int __result = 0;\
        do{ \
            if (dev_fd < 0) {    \
                __result = env_open();    \
            }    \
        }while(0);

    char * getFWParaConfig(const char * section_name,const char * key)
    {
        if (section_name == NULL || key == NULL)
        {
            return NULL;
        }

        CHECK_ENV_OPEN();
        if (__result != 0) {
           return NULL;
        }

        pthread_mutex_lock(&env_value_mutex);

        if (env_value_off >= (int)sizeof(env_value_buf)) {
            env_value_off = 0;
        }

        env_ioctl_args_t arg;
        memset(&arg, 0, sizeof(arg)); 
        strncpy(arg.section_name, section_name, sizeof(arg.section_name));
        strncpy(arg.key, key, sizeof(arg.key));
        arg.buf = env_value_buf + env_value_off;
        arg.maxlen = sizeof(env_value_buf) - env_value_off;

        int ret = ioctl(dev_fd, ENV_IOCGET, &arg);
        if (ret < 0 && errno != ENOBUFS) {
            pthread_mutex_unlock(&env_value_mutex);
            return NULL;
        }

        if (ret >= 0) {
            env_value_off += strlen(arg.buf) + 1;
            pthread_mutex_unlock(&env_value_mutex);
            return arg.buf;
        }

        // (errno == ENOBUFS):  space not enough
        arg.buf = env_value_buf;
        arg.maxlen = sizeof(env_value_buf);
        env_value_off = 0;

        ret = ioctl(dev_fd, ENV_IOCGET, &arg);
        if (ret != 0) {
            pthread_mutex_unlock(&env_value_mutex);
            return NULL;
        }

        env_value_off += strlen(arg.buf) + 1;

        pthread_mutex_unlock(&env_value_mutex);
        return arg.buf;
    }

    char * getFWParaConfig(const char * key)
    {
        if (key == NULL) {
            return NULL;
        }

        return getFWParaConfig("user",key);
    }

    int setFWParaConfig(const char * section_name,const char *key, const char *value, int overwrite)
    {
        if (section_name == NULL || key == NULL || value == NULL) {
            return -1;
        }

        CHECK_ENV_OPEN();
        if (__result != 0) {
           return -1;
        }

        env_ioctl_args_t arg;
        memset(&arg, 0, sizeof(arg)); 
        strncpy(arg.section_name, section_name, sizeof(arg.section_name));
        strncpy(arg.key, key, sizeof(arg.key));
        arg.buf = (char *)value;
        arg.maxlen = strlen(value)+1;
        arg.overwrite = overwrite;

        return ioctl(dev_fd, ENV_IOCSET, &arg);
    }

    int setFWParaConfig(const char *key, const char *value, int overwrite)
    {
        if (key == NULL || value == NULL) {
            return -1;
        }

        return setFWParaConfig("user",key,value,overwrite);
    }

    int saveFWParaConfig(void)
    {
        CHECK_ENV_OPEN();
        if (__result != 0) {
           return -1;
        }

        return ioctl(dev_fd, ENV_IOCSAVE, NULL);
    }

    int unsetFWParaConfig(const char * section_name,const char *key)
    {
        if (section_name == NULL || key == NULL) {
            return -1;
        }

        CHECK_ENV_OPEN();
        if (__result != 0) {
           return -1;
        }

        env_ioctl_args_t arg;
        memset(&arg, 0, sizeof(arg)); 
        strncpy(arg.section_name, section_name, sizeof(arg.section_name));
        strncpy(arg.key, key, sizeof(arg.key));

        return ioctl(dev_fd, ENV_IOCUNSET, &arg);
    }

    int unsetFWParaConfig(const char *key)
    {
        if (key == NULL) {
            return -1;
        }

        return unsetFWParaConfig("user",key);
    }

    int clearFWParaConfig(void)
    {
        CHECK_ENV_OPEN();
        if (__result != 0) {
           return -1;
        }

        return ioctl(dev_fd, ENV_IOCCLR, NULL);
    }
}
