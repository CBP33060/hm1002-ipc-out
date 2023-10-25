#ifndef __LOG_MANAGE_KERNEL_READ_H__
#define __LOG_MANAGE_KERNEL_READ_H__
#include <iostream>
#include <fstream>
#include "type_def.h"
#include "global_export.h"
#include <string>
#include <future>
#include <stdio.h>
#include <unistd.h>

namespace maix {

    class CLogManageKernelRead
    {
    public:
        CLogManageKernelRead();
        ~CLogManageKernelRead();

        using LogCallback = std::function<void(std::string)>;

        mxbool init();
        mxbool unInit();
        void registCallBack(LogCallback callBack);

    private:

        void run();

        std::ifstream m_ifKernelStream;

        mxbool m_bInit;
        mxbool m_bRun;

        std::thread m_threadKernelRead;

        LogCallback m_funCallback;

    };
}
#endif //__LOG_MANAGE_KERNEL_READ_H__