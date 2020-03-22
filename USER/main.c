#include "global.h"

 int main(void)
 {	 
	  
	  //u8 datatemp[2048];
	  //u32 FLASH_SIZE=128*1024*1024; 
		//u8 usbstatus=0;
		delay_init();	    	   //延时函数初始化	  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
		uart_init(115200);	 	//串口初始化为115200
		KEY_Init();
		TIM3_Init(9, 7199);    //系统定时器1kHz
		Adc_Init();		  		   //ADC初始化
		Dac1_Init();				   //DAC初始化
	 
		LCD_Init();
	 
		delay_ms(1800);
		USB_Port_Set(0); 	//USB先断开
		delay_ms(700);
		USB_Port_Set(1);	//USB再次连接
		Set_USBClock();   
		USB_Interrupts_Config();    
		USB_Init();
		W25QXX_Init();			//W25QXX初始化
		while(1)
		{		
			
//			if(usbstatus!=bDeviceState)//USB连接状态发生了改变.
//			{
//				usbstatus=bDeviceState;//记录新的状态
//				if(usbstatus==CONFIGURED)
//				{
//					POINT_COLOR=BLUE;
//					LCD_ShowString(30,130,200,16,16,"USB Connected    ");//提示USB连接成功
//					LED1=0;//DS1亮
//				}else
//				{
//					POINT_COLOR=RED;
//					LCD_ShowString(30,130,200,16,16,"USB disConnected ");//提示USB断开
//					LED1=1;//DS1灭
//				}
//			}
			//VoltagePrint();
			//VoltageIntervalPrint();
			KeyAction();

			writeFile();
			readFile();
			mainWhileCountPerSec();
			
			
		}
 }

