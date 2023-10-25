#include "log_manage_kernel_read.h"


#define KERNEL_READ_DEV_PATH                  "/dev/kmsg"     //kernel日志读取节点

namespace maix {
    CLogManageKernelRead::CLogManageKernelRead()
        : m_bInit(mxfalse)
        , m_bRun(mxfalse)
    {
        m_funCallback = NULL;
    }

    CLogManageKernelRead::~CLogManageKernelRead()
    {
        m_bRun = mxfalse;
    }

    mxbool CLogManageKernelRead::init()
    {

        m_threadKernelRead = std::thread([this]() {
            this->run();
        });
        m_threadKernelRead.detach();
        m_bInit = mxtrue;

        return mxtrue;
    }

    mxbool CLogManageKernelRead::unInit()
    {
        if(m_ifKernelStream)
        {
            m_ifKernelStream.close();
        }
        m_bRun = mxfalse;
        m_bInit = mxfalse;
        return mxtrue;
    }

    void CLogManageKernelRead::registCallBack(LogCallback callBack)
    {
        m_funCallback = callBack;
    }

    void CLogManageKernelRead::run()
    {
        m_bRun = mxtrue;
        while (m_bRun)
        {
            m_ifKernelStream.open(KERNEL_READ_DEV_PATH);
            if(m_ifKernelStream)
            {
                break;
            }
            else
            {
                if(m_funCallback)
                {
                    m_funCallback("kernel read open faild");
                }
                
            }
            usleep(1000 * 1000);
        }
        
        while (m_bRun)
        {
            std::string kernelLogData;
            if(getline(m_ifKernelStream,kernelLogData))
            {
                if(m_funCallback)
                {
                    m_funCallback(kernelLogData);
                }
            }
        }

        if(m_ifKernelStream)
        {
            m_ifKernelStream.close();
        }

    }


}