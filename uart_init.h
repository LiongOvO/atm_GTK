#ifndef _UART_INIT_H_
#define _UART_INIT_H_

#include <termios.h>

extern int uart_open(int *fd,char *port);
extern void uart_close(int fd);  
extern int uart_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
extern int uart_init(int fd);


#endif
