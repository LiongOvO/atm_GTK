#ifndef __RFID_LIB_H
#define __RFID_LIB_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>
#include <termios.h>


#include "uart_init.h"
#include "s5p6818_gpio.h"

//函数接口
extern unsigned char reg_set(unsigned char reg_addr, unsigned char value);
extern unsigned char reg_get(unsigned char reg_addr);
extern void rfid_carda_init(void);
extern void rfid_cardb_init(void);
extern unsigned char rfid_request(void);
extern unsigned char rfid_carda_request(unsigned char req_code, unsigned char *card_type);
extern unsigned char rfid_anticoll(unsigned char *card_num);
extern unsigned char rfid_attrib(void);
extern unsigned char rfid_get_uid(unsigned char *uid_buf);
extern int get_rfid_card_id(unsigned char *id,char *type);
extern void uart_rfid_init(char *dev);

//通用标志
#define OK	0
#define NOK	1

//READ等待时间us
#define WAIT_USEC				10000

//天线使能
#define ANTENNA_ON				0
#define ANTENNA_OFF				1

//串口
#define UART_PUT_SUCCESS		0x00
#define UART_PUT_FAIL			0x01
#define UART_GET_SUCCESS		0x02
#define UART_GET_FAIL			0x03

//读写寄存器
#define WR_SUCCESS				0x04
#define WR_FAIL					0x05
#define RD_SUCCESS				0x06
#define RD_FAIL					0x07
#define SET_BIT_SUCCESS			0x08
#define SET_BIT_FAIL			0x09
#define CLR_BIT_SUCCESS			0x0A
#define CLR_BIT_FAIL			0x0B

//读写器通信
#define COMMAND_ERR				0x0C
#define CMD_SUCCESS				0x0D
#define CMD_FAIL				0x0E

//读写命令
#define PCD_IDLE                 (0x00)           	// 取消当前命令
#define PCD_AUTHENT              (0x0E)          	// 验证密钥
#define PCD_RECEIVE              (0x08)       		// 接收数据
#define PCD_TRANSMIT             (0x04)             // 发送数据
#define PCD_TRANSCEIVE           (0x0C)             // 发送并接收数据
#define PCD_RESETPHASE           (0x0F)             // 复位
#define PCD_CALCCRC              (0x03)             // CRC计算

//Mifare_One卡片命令字
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠
#define NOP         0xFF

//MF522寄存器定义
// PAGE 0
#define     RFU00                 0x00
#define     CommandReg            0x01    //启动或停止正在执行的命令
#define     ComIEnReg             0x02    //中断请求传递使能/禁止控制寄存器
#define     DivlEnReg             0x03    //中断请求传递使能/禁止控制寄存器
#define     ComIrqReg             0x04    //包含中断请求标志
#define     DivIrqReg             0x05	  //包含中断请求标志
#define     ErrorReg              0x06    //错误标志，指示执行上次指令的错误状态
#define     Status1Reg            0x07    //包含通讯的状态标志
#define     Status2Reg            0x08    // 包含接收器和发射器的状态标志
#define     FIFODataReg           0x09	  // 64位FIFO数据输入输出寄存器
#define     FIFOLevelReg          0x0A	  // 当前FIFO内的byte数
#define     WaterLevelReg         0x0B	  // 定义FIFO上溢和下溢报警FIFO深度
#define     ControlReg            0x0C	   //不同的控制寄存器
#define     BitFramingReg         0x0D	  //面向位的帧的调节
#define     CollReg               0x0E	  //侦查到第一个冲突位的位置
#define     RFU0F                 0x0F	  //
// PAGE 1
#define     RFU10                 0x10	  //
#define     ModeReg               0x11	  // 定义发送和接收的常用模式
#define     TxModeReg             0x12	  //  定义发送过程中的数据传输速率
#define     RxModeReg             0x13	  //  定义接收过程中的数据传输速率
#define     TxControlReg          0x14	  //   控制发射天线管脚TX1，TX2的逻辑特性
#define     TxASKReg             0x15	  //   控制天线驱动器的设置
#define     TxSelReg              0x16	  //   控制天线驱动器的内部源
#define     RxSelReg              0x17	  // 选择内部的接收器设置
#define     RxThresholdReg        0x18	  // 选择位译码器的阀值
#define     DemodReg              0x19	   // 定义解调器的设置
#define     RFU1A                 0x1A	   //
#define     RFU1B                 0x1B	   //
#define     MifareReg             0x1C	   //  控制ISO14443/MIFARE模式中106kbits通信
#define     RFU1D                 0x1D	   //
#define     RFU1E                 0x1E		//
#define     SerialSpeedReg        0x1F		//选择串行接口的速率
// PAGE 2
#define     RFU20                 0x20     //
#define     CRCResultRegM         0x21		//
#define     CRCResultRegL         0x22		//
#define     RFU23                 0x23		//
#define     ModWidthReg           0x24		// 控制MODWIDTH的设置
#define     RFU25                 0x25		//
#define     RFCfgReg              0x26		// 配置接收器增益
#define     GsNReg                0x27		//	选择发射脚TX1和TX2发射天线的调制电导
#define     CWGsCfgReg            0x28		// 选择发射脚TX1和TX2发射天线的调制电导
#define     ModGsCfgReg           0x29		// 选择发射脚TX1和TX2发射天线的调制电导
#define     TModeReg              0x2A		// 定义内部定时器设置
#define     TPrescalerReg         0x2B		//
#define     TReloadRegH           0x2C		// 描述16位定时器重装值
#define     TReloadRegL           0x2D		//
#define     TCounterValueRegH     0x2E		//  显示16位定时器实际值
#define     TCounterValueRegL     0x2F		//
// PAGE 3
#define     RFU30                 0x30
#define     TestSel1Reg           0x31	   //常用测试信号配置
#define     TestSel2Reg           0x32	   //常用测试信号配置和PRBS控制
#define     TestPinEnReg          0x33	   //
#define     TestPinValueReg       0x34	   //
#define     TestBusReg            0x35	   //  显示内部测试总线的状态
#define     AutoTestReg           0x36	   //  控制数字自显示
#define     VersionReg            0x37	   //  显示版本
#define     AnalogTestReg         0x38	   //  控制管脚AUX1，AUX2
#define     TestDAC1Reg           0x39     //  定义TestDAC1的测试值
#define     TestDAC2Reg           0x3A     //  定义TestDAC2的测试值
#define     TestADCReg            0x3B     // 显示ADCi和Q通道的实际值
#define     RFU3C                 0x3C     //
#define     RFU3D                 0x3D     //
#define     RFU3E                 0x3E     //
#define     RFU3F		  0x3F
#define MAXRLEN 18 


#endif

