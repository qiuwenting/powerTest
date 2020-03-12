#include "global.h"

								  
u32 KEY0_count = 0;//����0
u8 KEY0_first_state = 0;
u8 KEY0_down_state = 0;
u8 KEY0_up_state = 0;

u32 KEY1_count = 0;//����1
u8 KEY1_first_state = 0;
u8 KEY1_down_state = 0;
u8 KEY1_up_state = 0;

u32 KEY2_count = 0;//����2
u8 KEY2_first_state = 0;
u8 KEY2_down_state = 0;
u8 KEY2_up_state = 0;



//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
	
#ifdef DEBUG_BOARD
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8|GPIO_Pin_9;//KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;//KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOD, &GPIO_InitStructure);//
	
#else	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//KEY0-KEY2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
#endif
}

s8 ReadKeyValue(void)
{
	uint8_t Keyvalue = 0;

	//����0��ȥ����ȡ
	if((KEY0_first_state == 0) && (KEY0_down_state == 0) && (KEY0_up_state == 0))
	{
		if (!KEY0)
		{
			KEY0_first_state = 1;
			KEY0_count = 0;
		}
	}
	
	if (KEY0_first_state == 1 && KEY0_count > 20)
	{
		if (!KEY0)
		{
			KEY0_down_state = 1;
		}
		KEY0_first_state = 0;
	}
	
	if (KEY0_down_state == 1)
	{
		if (KEY0)
		{
			KEY0_up_state = 1;
			KEY0_down_state = 0;
			KEY0_count = 0;
		}
		if (KEY0_count > 1000)
		{
			KEY0_count = 980;//����Ϊ950��˼�ǳ�����ÿ����100ms����һ�ΰ���
			Keyvalue = 0;
			return Keyvalue;
		}
	}
	
	if (KEY0_up_state == 1 && KEY0_count > 20)
	{
		KEY0_up_state = 0;
		Keyvalue = 0;
		return Keyvalue;		
	}
	//-------------------------------------------------------------------------//
	//����1��ȥ����ȡ
	if((KEY1_first_state == 0) && (KEY1_down_state == 0) && (KEY1_up_state == 0))
	{
		if (!KEY1)
		{
			KEY1_first_state = 1;
			KEY1_count = 0;
		}
	}
	
	if (KEY1_first_state == 1 && KEY1_count > 20)
	{
		if (!KEY1)
		{
			KEY1_down_state = 1;
		}
		KEY1_first_state = 0;
	}
	
	if (KEY1_down_state == 1)
	{
		if (KEY1)
		{
			KEY1_up_state = 1;
			KEY1_down_state = 0;
			KEY1_count = 0;
		}
		if (KEY1_count > 1000)
		{
			KEY1_count = 980;//����Ϊ950��˼�ǳ�����ÿ����100ms����һ�ΰ���
			Keyvalue = 1;
			return Keyvalue;
		}
	}
	
	if (KEY1_up_state == 1 && KEY1_count > 20)
	{
		KEY1_up_state = 0;
		Keyvalue = 1;
		return Keyvalue;		
	}
	//-------------------------------------------------------------------------//
		//����2��ȥ����ȡ
	if((KEY2_first_state == 0) && (KEY2_down_state == 0) && (KEY2_up_state == 0))
	{
		if (!KEY2)
		{
			KEY2_first_state = 1;
			KEY2_count = 0;
		}
	}
	
	if (KEY2_first_state == 1 && KEY2_count > 20)
	{
		if (!KEY2)
		{
			KEY2_down_state = 1;
		}
		KEY2_first_state = 0;
	}
	
	if (KEY2_down_state == 1)
	{
		if (KEY2)
		{
			KEY2_up_state = 1;
			KEY2_down_state = 0;
			KEY2_count = 0;
		}
		if (KEY2_count > 1000)
		{
			KEY2_count = 980;//����Ϊ950��˼�ǳ�����ÿ����100ms����һ�ΰ���
			Keyvalue = 2;
			return Keyvalue;
		}
	}
	
	if (KEY2_up_state == 1 && KEY2_count > 20)
	{
		KEY2_up_state = 0;
		Keyvalue = 2;
		return Keyvalue;		
	}
	
	return -1;	
}

void KEY0_Action(void)
{
	Dac1_Add_Val(10);
	return;
}

void KEY1_Action(void)
{
	Dac1_Sub_Val(10);
	return;
}

void KEY2_Action(void)
{
	usb_printf("usb test\n");
	return;
}

void KeyAction(void)
{
	u8 keyvalue = 0;
	keyvalue = ReadKeyValue();
	switch (keyvalue)
	{
		case 0:
		{
			KEY0_Action();
			break;
		}
		case 1:
		{
			KEY1_Action();
			break;
		}
		case 2:
		{
			KEY2_Action();
			break;
		}
		
		default:
			break;
	}
}



