#ifndef __KEY_H
#define __KEY_H	 

#include "global.h" 


//#define KEY0 PEin(4)   	//PE4
//#define KEY1 PEin(3)	//PE3 
//#define KEY2 PEin(2)	//PE2
//#define WK_UP PAin(0)	//PA0  WK_UP



#ifdef DEBUG_BOARD

#define KEY0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)//��ȡ����0
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)//��ȡ����1
#define KEY2  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_6)//��ȡ����2

#else

#define KEY0	PEin(4) //PE4
#define KEY1	PEin(3)	//PE3 
#define KEY2 	PEin(2)	//PE2
#define WK_UP 	PAin(0)	//PA0  WK_UP��KEY_UP


#endif
 

#define KEY0_PRES 	1	//KEY0����
#define KEY1_PRES	  2	//KEY1����
#define KEY2_PRES	  3	//KEY2����
#define WKUP_PRES   4	//KEY_UP����(��WK_UP/KEY_UP)


extern u32 KEY0_count;//����1
extern u32 KEY1_count;//����2
extern u32 KEY2_count;//����3


extern void KEY_Init(void);//IO��ʼ��			
extern void KeyAction(void);//ɨ��+��������
#endif
