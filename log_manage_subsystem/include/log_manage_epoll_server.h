#ifndef __LOG_MANAGE_EPOLL_SERVER_H__
#define __LOG_MANAGE_EPOLL_SERVER_H__
#include "type_def.h"
#include "global_export.h"
#include <string>
#include <map>
#include <mutex>
#include <future>
#include <sys/epoll.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

namespace maix {

    typedef enum
    {
        TYPE_LOG_NORMAL,
        TYPE_LOG_MCU,
    }E_EPOLL_LOG_TYPE;

    class CLogManageEpollServer
    {
    public:
        CLogManageEpollServer();
        ~CLogManageEpollServer();

        using LogCallback = std::function<void(E_EPOLL_LOG_TYPE,std::string)>;

        mxbool init(std::string unixPath);
        mxbool unInit();
        void registCallBack(LogCallback callBack);

    private:

        mxbool createSocketServer(std::string unixPath);
        mxbool createEpoll();
        void run();
        void handleEpollEvents(epoll_event* events,int epollWaiteFd);
        void logDataCallBack(E_EPOLL_LOG_TYPE enmType,std::string logData);

        int m_iSocketFd;
        int m_iEpollFd;
        int m_iEpollWaiteFd;

        mxbool m_bInit;
        mxbool m_bRun;

        std::thread m_threadAccept;

        LogCallback m_funCallback;

    };
}
#endif //__LOG_MANAGE_EPOLL_SERVER_H__