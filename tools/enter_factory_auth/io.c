#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>  
#include <errno.h>  
#include <arpa/inet.h>  
#include <signal.h>
#include <stdarg.h>

int recv_timeout(int fd,void *buf, int len, int timeout_ms)
{
    if(fd < 0)
    {
        return -1;
    }
    fd_set rset;
    int ret = 0;
    struct timeval t;
    size_t read_len = 0;

    int tims = timeout_ms / 10;

    while (read_len < len)
    {
        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        t.tv_sec = 0;
        t.tv_usec = 10*1000;
        ret = select(fd + 1, &rset, NULL, NULL, &t);
        if (ret <= 0) {
            if (errno == EINTR) 
            {
                // 信号中断
                continue;
            }

            tims--;
            // 超时（单次接收时间间隔超时）但已经接收到一些数据了
            if (ret == 0 && read_len > 0) 
            {   
                break;
            }
            
            // 超出设定接收时间了
            if(tims <= 0)
            {
                break;
            }

            if(ret < 0)
            {
                return ret;
            }
        } 
        else
        {
            ret = read(fd, (char *)buf + read_len, len - read_len);
            if (ret < 0)
            {
                return ret;
            }
            else
            {
                read_len += ret;
            }
        }
    }
    return read_len;
}

// int recv_timeout(int fd,void *buf, int len, int timeout_ms)
// {
//     int ret;
//     size_t  rsum = 0;
//     ret = 0;
//     fd_set rset;
//     struct timeval t;
//     int tims = timeout_ms / 10;
//     while (rsum < len)
//     {
//         t.tv_sec = 0;
//         t.tv_usec = 10*1000;
//         FD_ZERO(&rset);
//         FD_SET(fd, &rset);
//         ret = select(fd+1, &rset, NULL, NULL, &t);
//         if (ret <= 0) {
//             if (ret == 0) 
//             {   
//                 if(rsum != 0)
//                 {
//                     return rsum;
//                 }
//                 else
//                 {
//                     //timeout
//                     return -1;
//                 }
//             }
//             if (errno == EINTR) 
//             {
//                 // 信号中断
//                 continue;
//             }
//             return -errno;
//         } 
//         else
//         {
//             ret = read(fd, (char *)buf + rsum, len - rsum);
//             if (ret <= 0)
//             {
//                 if(ret != 0){
//                     printf("read error: %d\n", ret);
//                 }
//                 return ret;
//             }
//             else
//             {
//                 rsum += ret;
//             }
//         }
//     }

//     return rsum;
// }

int send_buf(int fd, char *buf, int len)
{
    return write(fd,buf,len);
}

int set_uart_opt(int fd,int bits,char parity,int baudrate,int stopbits)
{
    if(fd < 0)
    {
        return -1;
    }

    struct termios newtio,oldtio;

    if  (tcgetattr(fd,&oldtio)  !=  0)
    { 
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));

    /*CREAD 开启串行数据接收，CLOCAL并打开本地连接模式*/
    newtio.c_cflag  |=  CLOCAL | CREAD;

    /*设置数据位*/
    newtio.c_cflag &= ~CSIZE;
    switch(bits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    /* 设置奇偶校验位 */
    switch(parity)
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': 
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':  
        newtio.c_cflag &= ~PARENB;
        break;
    }
    
    /* 设置波特率 */
    switch(baudrate)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    }

    /*设置停止位*/
    if(stopbits == 1)
        newtio.c_cflag &=  ~CSTOPB;
    else if (stopbits == 2)
        newtio.c_cflag |=  CSTOPB;
    
    newtio.c_iflag &= ~(IXON);
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;

    tcflush(fd,TCIFLUSH);
    newtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_oflag  &= ~OPOST;

    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        return -1;
    }

    return 0;
}