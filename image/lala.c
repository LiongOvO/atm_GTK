#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "rfid_lib.h"

//串口号
#define UART_DEV  	"/dev/ttySAC2"

int main(int argc, char const *argv[])
{
	shuaka_function();
	return 0;
}

void shuaka_function(void)
{

	int len,i;char type;
	//len:读取id的长度   type：表示你的卡类型
	unsigned char id[18] = {0};
	//id就是卡号
	//捕获信号
	uart_rfid_init(UART_DEV);//设备初始化
	char hehe[200] = "";
	while(1)
	{
		if(len = get_rfid_card_id(id,&type)){
			
			
			// printf("%c类卡卡号:",type);
			//------------打印卡号----------------
			// for(i=0;i<len;i++)
			// 	printf("%02x ",id[i]);			
			for (i = 0; i < len; ++i)
			{
				printf("0x%02x", id[i]);
				
			}
			if (len == 4)
			{
				sprintf(hehe, "%02x %02x %02x %02x", id[0],id[1],id[2],id[3]);
				// printf("---%s===\n", hehe);
			}
			else
			{
				sprintf(hehe, "%02x %02x %02x %02x %02x %02x %02x %02x", id[0],id[1],id[2],id[3], id[4],id[5],id[6],id[7]);
				// printf("---%s===\n", hehe);
			}
		}
	}
	return;
}