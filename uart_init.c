#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>

/**
	功能：打开串口函数
	参数fd：文件描述符
	参数port：串口路径
	返回值：成功 串口文件描述符
			失败 -1
*/
int uart_open(int *fd, char* port)
{
	*fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (-1 == *fd)
	{
		perror("Can't Open Serial Port");
		return -1;
	}

	/*将串口恢复成阻塞,打开的时候是非阻塞的*/
/*	if(fcntl(*fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return -1;
	}
 */
	/*测试是否为终端设备*/
	if(0 == isatty(STDIN_FILENO))
	{
		printf("standard input is not a terminal device\n");
		return -1;
	}

	return *fd;
}

/**
	功能：关闭串口
	参数：无
	返回值：无
*/
void uart_close(int fd)
{
	printf("doing close !\n");
	close(fd);
}

/**
	功能：串口常规配置接口uart
	参数fd：串口文件描述符
	参数speed：串口波特率{115200,  19200,  9600,  4800,  2400,  1200,  300};
	参数flow_ctrl：流控 0：不启用流控  1：硬件流控  2：软件流控
	参数databits：串口传输数据位 5,6,7,8
	参数stopbits：串口传输数据停止位 1：一位停止位  2：两位停止位
	参数parity：串口传输数据校验位  n：不使用奇偶校验   o：奇校验   e：偶校验
	返回值：成功 0
			失败 -1
*/
int uart_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{

	int   i = 0;
	int   speed_arr[] = { B38400, B115200, B19200, B9600, B4800, B2400, B1200, B300};
	int   name_arr[] = { 38400, 115200, 19200,	9600,  4800,  2400,  1200,	300};

	struct termios options;
	memset(&options, 0, sizeof(options));

	/*保留旧的串口配置，在原有的基础上更新配置*/
	if(tcgetattr( fd,&options) != 0)
	{
		perror("SetupSerial 1");
		return -1;
	}

	/*关闭不必要的配置，配置成原始模式，关闭软件流控*/
	options.c_iflag &= ~(IGNBRK | BRKINT | INLCR | ICRNL| IUCLC | INPCK | ISTRIP | IXON | IXANY | IXOFF);
	options.c_iflag &= ~(ICRNL | IXON);//接收\r\n
	options.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL);
	options.c_cflag &= ~(CSIZE | PARENB | HUPCL);
	options.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG | ECHOE | ECHOK | ECHOCTL | ECHOKE|ISIG);
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;
	options.c_cflag |= CLOCAL | CREAD;

	/*波特率*/
 	for (i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	/*流控*/
    switch(flow_ctrl)
    {
		case 0 :
              options.c_cflag &= ~CRTSCTS;
              break;

		case 1 :
              options.c_cflag |= CRTSCTS;
              break;
		case 2 :
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }

	/*数据位*/
    switch (databits)
    {
		case 5:
                 options.c_cflag |= CS5;
                 break;
		case 6:
                 options.c_cflag |= CS6;
                 break;
		case 7:
				options.c_cflag &= ~CSIZE;
                 options.c_cflag |= CS7;
                 break;
		case 8:
				options.c_cflag &= ~CSIZE;
                 options.c_cflag |= CS8;
                 break;
		default:
                 fprintf(stderr,"Unsupported data size\n");
                 return -1;
    }

	/*停止位*/
	switch (stopbits)
    {
		case 1:
                	options.c_cflag &= ~CSTOPB; break;
		case 2:
					options.c_cflag |= CSTOPB; break;
		default:
					fprintf(stderr,"Unsupported stop bits\n");
					return -1;
    }

	/*校验位*/
    switch (parity)
    {
		case 'n':
		case 'N':
                 options.c_cflag &= ~PARENB;
                 options.c_iflag &= ~INPCK;
                 break;
		case 'o':
		case 'O':
                 options.c_cflag |= (PARODD | PARENB);
                 options.c_iflag |= INPCK;
                 break;
		case 'e':
		case 'E':
                 options.c_cflag |= PARENB;
                 options.c_cflag &= ~PARODD;
                 options.c_iflag |= INPCK;
                 break;
        default:
                 fprintf(stderr,"Unsupported parity\n");
                 return -1;
    }

	/*刷新串口输入输出缓冲区*/
    tcflush(fd,TCIOFLUSH);

	/*配置新的串口*/
    if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("com set error!\n");
		return -1;
	}

    return 0;
}

/**
	功能：串口初始化
	参数fd：串口文件描述符
	返回值	成功 0
			失败 -1

	补充：波特率 9600
		  流控   无
		  数据位 8bit
		  停止位 1bit
		  校验位 无
*/
int uart_init(int fd)
{
    if (uart_set(fd, 9600, 0, 8, 1, 'N') == -1)
	{
		return -1;
	}
    else
	{
		return  0;
	}
}


