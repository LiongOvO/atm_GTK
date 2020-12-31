/*
	功能：针对于ISO/IEC14443 TypeB类卡的RFID操作库
	读写器型号兼容：MFRC522,MFRC523,FM17522,FM17550等

	!注意：TypeA类卡与TypeB类卡不兼容，请选择相应的RFID操作库
	!补充：此库易于移植到任意平台
*/
#include "rfid_lib.h"

//调试开关
#define DEBUG_ECHO     	1

int uart_fd;

void delay_10ms(unsigned int _10ms)
{
	usleep(_10ms*10000);
}

/**
	功能：向串口发送一个字符
	参数ch：待发送的字符
	返回值：成功写入返回 0x00
			失败写入返回 0x01
*/
unsigned char uart_putchar(unsigned char ch)
{
	unsigned char  status = 0;

   	status = write(uart_fd, &ch, 1);
	if(status > 0)
		return UART_PUT_SUCCESS;
	else
		return UART_PUT_FAIL;
}

/**
	功能：向串口接收一个字符
	参数ch：待接收的字符
	参数wait_usec：等待n微秒
	返回值：成功读入返回 0x02
			失败读入返回 0x03
*/
unsigned char uart_getchar(unsigned char *ch, unsigned int wait_usec)
{
	struct timeval tv;
	fd_set set;

 	FD_ZERO(&set);
	FD_SET(uart_fd, &set);
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 0;
	tv.tv_usec = wait_usec;

 	if(select(uart_fd+1, &set, NULL, NULL, &tv)>0)
	{
 		if(FD_ISSET(uart_fd, &set))
		{
			read(uart_fd, ch, 1);
			return UART_GET_SUCCESS;
		}
	}

	return UART_GET_FAIL;
}

/**
	功能：指定寄存器写
	参数reg_addr：待写入寄存器的地址
	参数value：待写入寄存器的值
	返回值：成功写入返回 0x04
			失败写入返回 0x05
	补充：读写器串口协议：A0 A1 A2 A3 A4 A5 RFU RD/WR
		  RFU保留 0
		  RD读 1
		  WR写 0
*/
	unsigned char reg_set(unsigned char reg_addr, unsigned char value)
	{
		unsigned char ch = 0;
		unsigned char status = 0;
	
		reg_addr &= 0x3f;	//第7位写——置0，第6位保留——置0
		status = uart_putchar(reg_addr);
		if(status == UART_PUT_SUCCESS)
		{
			status = uart_putchar(value);
			if(status == UART_PUT_SUCCESS)
			{
				 status = uart_getchar(&ch, WAIT_USEC);
			}
			else
				return WR_FAIL;
		}
		else
			return WR_FAIL;
	
		if((status == UART_GET_SUCCESS) && (ch == reg_addr))
			return WR_SUCCESS;
		else
			return WR_FAIL;
	}
	
	/**
		功能：指定寄存器读
		参数uart_fd：串口文件描述符
		参数reg_addr：待读取寄存器的地址
		返回值：成功读入返回 成功字符
				失败读入返回 0xFF
		补充：读写器串口协议：A0 A1 A2 A3 A4 A5 RFU RD/WR
			  RFU保留 0
			  RD读 1
			  WR写 0
	*/
	unsigned char reg_get(unsigned char reg_addr)
	{
		unsigned char ch = 0;
		unsigned char status = 0;
	
		reg_addr = (reg_addr & 0x3f) | 0x80;	//第7位写——置1，第6位保留——置0
		status = uart_putchar(reg_addr);
		if(status == UART_PUT_SUCCESS)
		{
			status = uart_getchar(&ch, WAIT_USEC);
		}
		if(status == UART_GET_FAIL)
			return 0xFF;
		return ch;
	}
	/**
		功能：指定寄存器的某bit位进行置位
		参数uart_fd：串口文件描述符
		参数reg_addr：待置位寄存器的地址
		参数mask：待置位的bit位
		返回值：成功置位 0x08
				失败置位 0x09
	*/
	unsigned char reg_set_bit(unsigned char reg_addr, unsigned char mask)
	{
		unsigned char status = 0;
	
		status = reg_set(reg_addr, reg_get(reg_addr) | mask);
		if(status == WR_SUCCESS)
			return SET_BIT_SUCCESS;
		else
			return SET_BIT_FAIL;
	}
	
	/**
		功能：指定寄存器的某bit位进行屏蔽
		参数uart_fd：串口文件描述符
		参数reg_addr：待屏蔽寄存器的地址
		参数mask：待屏蔽的bit位
		返回值：成功屏蔽 0x0A
				失败屏蔽 0x0B
	*/
	unsigned char reg_clr_bit(unsigned char reg_addr, unsigned char mask)
	{
		unsigned char status = 0;
	
		status = reg_set(reg_addr, reg_get(reg_addr) & ~mask);
		if(status == WR_SUCCESS)
			return CLR_BIT_SUCCESS;
		else
			return CLR_BIT_FAIL;
	}
	
	/////////////////////////////////////////////////////////////////////
	//功	 能：通过RC522和ISO14443卡通讯
	//参数说明：Command[IN]:RC522命令字
	//			pInData[IN]:通过RC522发送到卡片的数据
	//			InLenByte[IN]:发送数据的字节长度
	//			pOutData[OUT]:接收到的卡片返回数据
	//			*pOutLenBit[OUT]:返回数据的位长度
	/////////////////////////////////////////////////////////////////////
	unsigned char rfid_cmd(unsigned char Command, 
					 unsigned char *pInData, 
					 unsigned char InLenByte,
					 unsigned char *pOutData, 
					 unsigned int  *pOutLenBit)
	{
		char status = CMD_FAIL;
		unsigned char irqEn   = 0x00;
		unsigned char waitFor = 0x00;
		unsigned char lastBits;
		unsigned char n;
		unsigned int i;
		switch (Command)
		{
		   case PCD_AUTHENT:
			  irqEn   = 0x12;
			  waitFor = 0x10;
			  break;
		   case PCD_TRANSCEIVE:
			  irqEn   = 0x77;
			  waitFor = 0x30;
			  break;
		   default:
			 break;
		}
	   
		reg_set(ComIEnReg,irqEn|0x80);
		reg_clr_bit(ComIrqReg,0x80);
		reg_set(CommandReg,PCD_IDLE);
		reg_set_bit(FIFOLevelReg,0x80);
		
		for (i=0; i<InLenByte; i++)
		{	reg_set(FIFODataReg, pInData[i]);	}
		reg_set(CommandReg, Command);
	   
		
		if (Command == PCD_TRANSCEIVE)
		{	 reg_set_bit(BitFramingReg,0x80);  }
		
	//	  i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
	 i = 2000;
		do 
		{
			 n = reg_get(ComIrqReg);
			 i--;
		}
		while ((i!=0) && !(n&0x01) && !(n&waitFor));
		reg_clr_bit(BitFramingReg,0x80);
			  
		if (i!=0)
		{	 
			 if(!(reg_get(ErrorReg)&0x1B))
			 {
				 status = CMD_SUCCESS;
				 if (n & irqEn & 0x01)
				 {	 status = COMMAND_ERR;	 }
				 if (Command == PCD_TRANSCEIVE)
				 {
					n = reg_get(FIFOLevelReg);
					lastBits = reg_get(ControlReg) & 0x07;
					if (lastBits)
					{	*pOutLenBit = (n-1)*8 + lastBits;	}
					else
					{	*pOutLenBit = n*8;	 }
					if (n == 0)
					{	n = 1;	  }
					if (n > MAXRLEN)
					{	n = MAXRLEN;   }
					for (i=0; i<n; i++)
					{	pOutData[i] = reg_get(FIFODataReg);	 }
				}
			 }
			 else
			 {	 status = CMD_FAIL;	}
			
	   }
	   
	
	   reg_set_bit(ControlReg,0x80); 		  // stop timer now
	   reg_set(CommandReg,PCD_IDLE); 
	   return status;
	}
	
	
	
	/**
		功能：天线操作
		参数uart_fd：串口文件描述符
		参数mode：ANTENNA_ON 	天线开
				  ANTENNA_OFF	天线关
		返回值：无
	*/
	void antenna_set(unsigned char mode)
	{
		if(mode == ANTENNA_ON)
		{
			if (!(reg_get((TxControlReg) & 0x03)))
			{
				reg_set_bit(TxControlReg, 0x03);   // 天线开
			}
		}
		else if(mode == ANTENNA_OFF)
		{
			reg_clr_bit(TxControlReg, 0x03);		// 天线关
		}
	}
	
	/**
		功能：定时器操作
		参数value：定时ms
		返回值：无
	
		补充：
			fm17550手册和mfrc522手册上定时器计算的公式不是完全一样
			因fm17550定时器时钟频率为13.56MHZ,而mfrc522定时器时钟频率为6.78MHZ
			但结果是大致相同的
	*/
	void timer_set(unsigned short value)
	{
		reg_set(TReloadRegL, (unsigned char)((value*10)&0xff)); 			//定时器初值
		reg_set(TReloadRegH, (unsigned char)(((value*10)>>8)&0xff));		//定时器初值
		/*分频后，一次计数100us*/
		/*分频作用bit为：TModeReg低两位+TPrescalerReg*/
		reg_set(TModeReg, 0x82);			//开启——读写器无线通信时定时器自动开启
		reg_set(TPrescalerReg, 0xA6);
	}
	
	/**
		功能：读写器初始化
		参数：无
		返回值：无
	*/
	void rfid_cardb_init(void)
	{
		/*复位*/
		reg_set(CommandReg, PCD_RESETPHASE);
		usleep(10*1000);				//复位需要时间
	
		/*大部分保持芯片软复位后的默认值即可*/
		reg_clr_bit(Status2Reg, 0x08);	//关闭加密传输
	
		/*发送部分*/
		reg_set(TxASKReg, 0x00);		//TypeB卡不需要100%ASK调制
		reg_set(TxModeReg, 0x03);		//发送数据帧格式调整为B类
		reg_set(ModGsCfgReg, 0x0F); 	//默认值为0x20,经调试0x08~0x0F可行
	
		/*接收部分*/
		reg_set(RFCfgReg, 0x7F);		//提高接收器的增益和灵敏度，主要用于增加通信距离，不配的话使用默认值也可以
		reg_set(RxModeReg, 0x03);		//接收数据帧格式调整为B类
	
		/*定时器——1ms*/
		timer_set(1);
	
		/*CRC*/
		reg_set(ModeReg, 0x3F);
	
		/*天线*/
		antenna_set(ANTENNA_OFF);
		usleep(10*1000);
		antenna_set(ANTENNA_ON);
	}
	
	/**
		功能：寻卡请求
		参数：无
		返回值：成功	OK
				失败	NOK
	*/
	unsigned char rfid_request(void)
	{
		unsigned char status = NOK;
		unsigned int len = 0;
		unsigned char buf[18] = {0};
	
		timer_set(5);		//定时器5ms
	//标准REQB:[Apf.AFI.PARAM.CRC16_B]
	//返回12字节卡类型:0x50 0x00 0x00 0x00 0x00 0xD1 0x03 0x86 0x0C 0x00 0x80 0x80
		buf[0] = 0x05;
		buf[1] = 0x00;
		buf[2] = 0x00;
	
		buf[3] = 0x71;
		buf[4] = 0xFF;
	
		status = rfid_cmd(PCD_TRANSCEIVE, buf, 5, buf, &len);
		if(status == CMD_SUCCESS)
		{
			status = OK;
		}
		else
			status = NOK;
	
		return status;
	}
	
	/**
		功能：ATTRIB
		参数：无
		返回值：成功	OK
				失败	NOK
	*/
	unsigned char rfid_attrib(void)
	{
		unsigned char status = NOK;
		unsigned int len = 0;
		unsigned char buf[18] = {0};
	
		timer_set(5);		//定时器5ms
	//非标ATTRIB:[0x1D.标识4B.参数3B.CID.CRC16_B]
	//选卡成功返回:0x08
		buf[0] = 0x1D;
	
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
	
		buf[5] = 0x00;
		buf[6] = 0x08;
		buf[7] = 0x01;
		buf[8] = 0x08;
	
		buf[9] = 0xF3;
		buf[10] = 0x10;
	
		status = rfid_cmd(PCD_TRANSCEIVE, buf, 11, buf, &len);
		if(status == CMD_SUCCESS)
		{
			status = OK;
		}
		else
			status = NOK;
	
		return status;
	}
	
	/**
		功能：得到UID
		参数uid_buf：UID
		返回值：成功	OK
				失败	NOK
	*/
	unsigned char rfid_get_uid(unsigned char *uid_buf)
	{
		unsigned char status = NOK;
		unsigned int len = 0;
		unsigned char buf[18] = {0};
	
		timer_set(5);		//定时器5ms
	//UID:[0x00.0x36.0x00.0x00.0x08.CRC16_B]
	//返回:x x x x x x x x 0x90 0x00(后两个字节为0x90 x00表示读取成功)
		buf[0] = 0x00;
		buf[1] = 0x36;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x08;
	
		buf[5] = 0x57;
		buf[6] = 0x44;
	
		status = rfid_cmd(PCD_TRANSCEIVE, buf, 7, buf, &len);
		if(status == CMD_SUCCESS)
		{
			memcpy(uid_buf, buf, 10);
			status = OK;
		}
		else
			status = NOK;
	
		return status;
	}
	/**
		功能：读写器初始化
		参数：无
		返回值：无
	*/
	void rfid_carda_init(void)
	{
		/*复位*/
		reg_set(CommandReg, PCD_RESETPHASE);
		usleep(10*1000);					//复位需要时间
	
		/*大部分保持芯片软复位后的默认值即可*/
		reg_clr_bit(Status2Reg, 0x08);		//关闭加密传输
		reg_set(ModeReg,0x3D);//add new
		reg_set(RxSelReg,0x86);//add new
	
		/*发送部分*/
		reg_set(TxASKReg, 0x40);			//TypeA卡需要100%ASK调制
	
		/*接收部分*/
		reg_set(RFCfgReg, 0x7F);			//提高接收器的增益和灵敏度，主要用于增加通信距离，不配的话使用默认值也可以
	
		/*定时器-1ms*/
		//timer_set(1); 
	
		reg_set(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		reg_set(TReloadRegH,0);
		reg_set(TModeReg,0x8D);
		reg_set(TPrescalerReg,0x3E);
		/*天线*/
		antenna_set(ANTENNA_OFF);
		usleep(10*1000);
		antenna_set(ANTENNA_ON);
	}
	
	/**
		功能：卡请求
		参数req_code：ALL = 0x52, IDLE = 0x26
		参数card_type: 返回卡类型（2 bytes）
		返回值：成功	OK
				失败	NOK
	*/
	unsigned char rfid_carda_request(unsigned char req_code, unsigned char *card_type)
	{
		unsigned char status = NOK;
		unsigned int len = 0;
		unsigned char buf[20] = {0};

		reg_clr_bit(Status2Reg, 0x08);//add new
		reg_set(BitFramingReg, 0x07);	//请求帧格式
		//timer_set(1);					//定时器1ms
		reg_set_bit(TxControlReg,0x03);//add new
	
		buf[0] = req_code;
		status = rfid_cmd(PCD_TRANSCEIVE, buf, 1, buf, &len);
	
		if ((status == CMD_SUCCESS) && (len == 16)) 		//2字节，16bit
		{
			*card_type	   = buf[0];
			*(card_type+1) = buf[1];
			status = OK;
		}
		else
		{
			status = NOK;
		}
	
		reg_set(BitFramingReg, 0x00);	//还原成标准帧格式
	
		return status;
	}
	
	/**
		功能：防冲突
		参数card_num：4字节卡序列号
		返回值：成功	OK
				失败	NOK
	*/
	unsigned char rfid_anticoll(unsigned char *card_num)
	{
		unsigned char status = OK;
		unsigned char i = 0, snr_check = 0;
		unsigned int len = 0;
		unsigned char buf[20] = {0};
	
		reg_clr_bit(Status2Reg, 0x08);//add new
		reg_set(BitFramingReg, 0x00);
		reg_clr_bit(CollReg, 0x80); //发生冲突位之后的所有的接收位被清除
		//timer_set(5);				//定时器5ms
	
		buf[0] = PICC_ANTICOLL1;				//s50卡，4字节，级联为1
		buf[1] = 0x20;
	
		status = rfid_cmd(PCD_TRANSCEIVE, buf, 2, buf, &len);
	
		if(status == CMD_SUCCESS)
		{
			 for(i=0; i<4; i++)
			 {
				 *(card_num+i) = buf[i];
				 snr_check	^= buf[i];
			 }
	
			 if(snr_check != buf[i])
			 {
				 status = NOK;
			 }
			 //status = OK;
			 return OK;
		}
		else
		{
			status = NOK;
		}
	
		reg_set_bit(CollReg, 0x80); //取消 发生冲突位之后的所有的接收位被清除
	
		return status;
	}

	void sig_dispose(int sig)
	{
		uart_close(uart_fd);
		puts("ctrl+c 打断退出");
		exit(0);
	}
	
	/**
		功能：对RST引脚的操作
		参数value：1 io高电平
				   0 io低电平
		返回值：无
	*/
	void do_rst(int value)
	{
		if(value == 0)
			system("echo 0 > /sys/class/gpio/gpio30/value");
		else if(value == 1)
			system("echo 1 > /sys/class/gpio/gpio30/value");
	}
	void uart_rest(char *dev)
	{
		//串口初始化
		uart_fd = uart_open(&uart_fd, dev);
		if(uart_fd<0)
		{
			printf("串口打开失败\n");
			exit(-1);
		}
		if(uart_init(uart_fd)<0)
		{
			printf("串口初始化失败\n");
			exit(-1);
		}
	}
	
	void uart_rfid_init(char *dev)
	{
		unsigned char version = 0;
		signal(SIGINT, sig_dispose);
		
		if(access("/sys/class/gpio/gpio30/value",F_OK))
			system("echo 30 > /sys/class/gpio/export");
	
		system("echo out > /sys/class/gpio/gpio30/direction");
		//外部复位RFID模块，直到成功读回正确版本号确定卡的存在
		do{
			uart_rest(dev);//如果版本读取失败还要重置串口
			do_rst(0);
			usleep(50*1000);
			do_rst(1);
			usleep(50*1000);
			version = reg_get(VersionReg);
	#if DEBUG_ECHO
			printf("version=%x\n", version);
	#endif
			if(version != 0x92 && version != 0x88){
				usleep(100*1000);
				uart_close(uart_fd);
				usleep(10*1000);
			}
		}while(version != 0x92 && version != 0x88);
	}
	
	/* 返回读取的卡号、类型及长度,返回0表示没读到设备 */
	int get_rfid_card_id(unsigned char *id,char *type)
	{
		unsigned char TagType[2];
		unsigned char buf[18] = {0};
		rfid_carda_init();/* 初始化A类卡 */
		if((rfid_carda_request(PICC_REQALL, TagType) == OK)&&(rfid_anticoll(buf) == OK))
		{
			*type = 'A';
			memcpy(id,buf,4);
			return 4;
		}
		rfid_cardb_init();/* 初始化B类卡 */
		if((rfid_request() == OK)&&(rfid_attrib() == OK)&&(rfid_get_uid(buf) == OK))
		{
			*type = 'B';
			memcpy(id,buf,8);
			return 8;
		}
		return 0;
	}
