#include "log_manage_epoll_server.h"
#include "cJSON.h"

#define LISTEN_MAX_NUM 20
#define EPOLL_SIZE 20
#define RECIVE_SIZE 1536        //1024 + 512 发送端日志为1024 512为json格式备用

#define MCU_LOG_PRINT_FLAG                  "-muc_log-"     //mcu打印log的标识

namespace maix {
    CLogManageEpollServer::CLogManageEpollServer()
        : m_bInit(mxfalse)
        , m_bRun(mxfalse)
    {
        m_iSocketFd = -1;
        m_iEpollFd = -1;
        m_iEpollWaiteFd = -1;
        m_funCallback = NULL;
    }

    CLogManageEpollServer::~CLogManageEpollServer()
    {
    }

    mxbool CLogManageEpollServer::init(std::string unixPath)
    {
        if(!createSocketServer(unixPath))
        {
            printf("createSocketServer err \n");
            return mxfalse;
        }
        if(!createEpoll())
        {
            printf("createEpoll err \n");
            return mxfalse;
        }

        m_threadAccept = std::thread([this]() {
            this->run();
        });
        m_bInit = mxtrue;

        return mxtrue;
    }

    mxbool CLogManageEpollServer::unInit()
    {
        close(m_iSocketFd);
        m_bRun = mxfalse;
        m_bInit = mxfalse;
        return mxtrue;
    }

    void CLogManageEpollServer::registCallBack(LogCallback callBack)
    {
        m_funCallback = callBack;
    }


    mxbool CLogManageEpollServer::createSocketServer(std::string unixPath)
    {
        int res = -1;
        sockaddr_un serverAddr;
        m_iSocketFd = socket(AF_UNIX,SOCK_DGRAM,0);
        if(m_iSocketFd < 0)
        {
            printf("socket failed : %s \n",strerror(errno));
            logDataCallBack(TYPE_LOG_NORMAL,"log manage socket failed");
            return mxfalse;
        }

        if(access(unixPath.c_str(),F_OK) == 0)
        {
            printf("unixPath has been exit \n");
            logDataCallBack(TYPE_LOG_NORMAL,"unixPath has been exit");
            unlink(unixPath.c_str());
        }
        bzero(&serverAddr,sizeof(serverAddr));
        serverAddr.sun_family = AF_UNIX;
        strcpy(serverAddr.sun_path,unixPath.c_str());

        res = bind(m_iSocketFd,(sockaddr *)&serverAddr,sizeof(serverAddr));
        if(res < 0)
        {
            close(m_iSocketFd);
            unlink(unixPath.c_str());
            printf("bind err: %s \n",strerror(errno));
            logDataCallBack(TYPE_LOG_NORMAL,"log bind err");
            return mxfalse;
        }
        // res = listen(m_iSocketFd,LISTEN_MAX_NUM);
        // if(res < 0)
        // {
        //     close(m_iSocketFd);
        //     unlink(unixPath.c_str());
        //     printf("listen err: %s \n",strerror(errno));
        //     return mxfalse;
        // }
        return mxtrue;
    }

    mxbool CLogManageEpollServer::createEpoll()
    {
        int res = -1;
        epoll_event socketFdEvent;
        bzero(&socketFdEvent,sizeof(socketFdEvent));
        socketFdEvent.data.fd = m_iSocketFd;
        socketFdEvent.events = EPOLLIN;
        m_iEpollFd = epoll_create(EPOLL_SIZE);
        if(m_iEpollFd < 0)
        {
            printf("epoll_create err: %s \n",strerror(errno));
            logDataCallBack(TYPE_LOG_NORMAL,"epoll_create err");
            close(m_iSocketFd);
            return mxfalse;
        }
        res = epoll_ctl(m_iEpollFd,EPOLL_CTL_ADD,m_iSocketFd,&socketFdEvent);
        if(res != 0)
        {
            printf("epoll_ctl socketFd err: %s \n",strerror(errno));
            logDataCallBack(TYPE_LOG_NORMAL,"epoll_ctl socketFd err");
            close(m_iSocketFd);
            return mxfalse;
        }
        return mxtrue;
    }

    void CLogManageEpollServer::run()
    {
        m_bRun = mxtrue;
        epoll_event epollEventArray[EPOLL_SIZE];
        while (m_bRun)
        {
            int epollWaitFd = -1;
            epollWaitFd = epoll_wait(m_iEpollFd,epollEventArray,EPOLL_SIZE,-1);
            if(epollWaitFd < 0)
            {
                printf("epoll_wait err");
                logDataCallBack(TYPE_LOG_NORMAL,"epoll_wait err");
                continue;
            }
            handleEpollEvents(epollEventArray,epollWaitFd);
        }
    }

    void CLogManageEpollServer::handleEpollEvents(epoll_event* events,int epollWaiteFd)
    {
        for (int i = 0; i < epollWaiteFd; i++)
        {
            // if(events[i].data.fd == m_iSocketFd)
            // {
            //     //有客户端连接
            //     printf("accept client \n");
            //     int acceptFd = -1;
            //     acceptFd = accept(m_iSocketFd,NULL,NULL);
            //     if(acceptFd < 0)
            //     {
            //         printf("accept err: %s \n",strerror(errno));
            //         continue;
            //     }
            //     epoll_event clientEvent;
            //     clientEvent.data.fd = acceptFd;
            //     clientEvent.events = EPOLLIN;
            //     epoll_ctl(m_iEpollFd,EPOLL_CTL_ADD,acceptFd,&clientEvent);
            // }
            // else 
            if(events[i].events & EPOLLIN)
            {
                char readBuf[RECIVE_SIZE] = {0};
                int res = read(events[i].data.fd,readBuf,sizeof(readBuf));
                if(res > 0)
                {
                    // printf("server receive %d data : %s \n" ,events[i].data.fd , readBuf);
                    std::string logData = std::string(readBuf);
                    std::string mcuHead = MCU_LOG_PRINT_FLAG;
                    std::string headLog = logData.substr(0,mcuHead.length());
                    if(mcuHead.compare(headLog) == 0)
                    {
                        std::string endLog = logData.substr(mcuHead.length());
                        logDataCallBack(TYPE_LOG_MCU,endLog);
                    }
                    else
                    {
                        logDataCallBack(TYPE_LOG_NORMAL,logData);
                    } 
                }
                else if(res <= 0)
                {
                    close(events[i].data.fd);
                    printf("client close %d \n",events[i].data.fd);
                    epoll_event clientEvent;
                    clientEvent.data.fd = events[i].data.fd;
                    clientEvent.events = EPOLLIN;
                    epoll_ctl(m_iEpollFd,EPOLL_CTL_DEL,events[i].data.fd,&clientEvent);
                }
            }
        }
        
    }

    void CLogManageEpollServer::logDataCallBack(E_EPOLL_LOG_TYPE enmType,std::string logData)
    {
        if(m_funCallback)
        {
            m_funCallback(enmType,logData);                      
        }
    }

}