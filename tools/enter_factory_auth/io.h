#ifndef __IO__H__
#define __IO__H__

int recv_timeout(int fd,void *buf, int len, int timeout_ms);
int send_buf(int fd, char *buf, int len);
int set_uart_opt(int fd,int bits,char parity,int baudrate,int stopbits);

#endif /* __IO__H__ */