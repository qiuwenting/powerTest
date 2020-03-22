#include "global.h"

 int main(void)
 {	 
	  
	  //u8 datatemp[2048];
	  //u32 FLASH_SIZE=128*1024*1024; 
		//u8 usbstatus=0;
		delay_init();	    	   //��ʱ������ʼ��	  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
		uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
		KEY_Init();
		TIM3_Init(9, 7199);    //ϵͳ��ʱ��1kHz
		Adc_Init();		  		   //ADC��ʼ��
		Dac1_Init();				   //DAC��ʼ��
	 
		LCD_Init();
	 
		delay_ms(1800);
		USB_Port_Set(0); 	//USB�ȶϿ�
		delay_ms(700);
		USB_Port_Set(1);	//USB�ٴ�����
		Set_USBClock();   
		USB_Interrupts_Config();    
		USB_Init();
		W25QXX_Init();			//W25QXX��ʼ��
		while(1)
		{		
			
//			if(usbstatus!=bDeviceState)//USB����״̬�����˸ı�.
//			{
//				usbstatus=bDeviceState;//��¼�µ�״̬
//				if(usbstatus==CONFIGURED)
//				{
//					POINT_COLOR=BLUE;
//					LCD_ShowString(30,130,200,16,16,"USB Connected    ");//��ʾUSB���ӳɹ�
//					LED1=0;//DS1��
//				}else
//				{
//					POINT_COLOR=RED;
//					LCD_ShowString(30,130,200,16,16,"USB disConnected ");//��ʾUSB�Ͽ�
//					LED1=1;//DS1��
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

