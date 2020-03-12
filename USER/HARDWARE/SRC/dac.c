#include "global.h"

void Dac1_Init(void)
{
  
	GPIO_InitTypeDef GPIO_InitStructure;
	DAC_InitTypeDef DAC_InitType;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	  //ʹ��PORTAͨ��ʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE );	  //ʹ��DACͨ��ʱ�� 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 // �˿�����
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		 //ģ������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4)	;//PA.4 �����
					
	DAC_InitType.DAC_Trigger=DAC_Trigger_None;	//��ʹ�ô������� TEN1=0
	DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_None;//��ʹ�ò��η���
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;//���Ρ���ֵ����
	DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Disable ;	//DAC1�������ر� BOFF1=1
  DAC_Init(DAC_Channel_1,&DAC_InitType);	 //��ʼ��DACͨ��1

	DAC_Cmd(DAC_Channel_1, ENABLE);  //ʹ��DAC1
  
  DAC_SetChannel1Data(DAC_Align_12b_R, 0);  //12λ�Ҷ������ݸ�ʽ����DACֵ

}

//����ͨ��1�����ѹ
//vol:0~3300,����0~3.3V
void Dac1_Set_Vol(u16 vol)
{
	u32 temp=vol;
	temp = temp * 4096 / 3300;
	DAC_SetChannel1Data(DAC_Align_12b_R,temp);//12λ�Ҷ������ݸ�ʽ����DACֵ
}

void Dac1_Add_Val(u16 val)
{
	u16 currentAdcValue;
	currentAdcValue = DAC_GetDataOutputValue(DAC_Channel_1);
	
	if((0x1000 - currentAdcValue) > val)
	{
		DAC_SetChannel1Data(DAC_Align_12b_R, currentAdcValue + val);//12λ�Ҷ������ݸ�ʽ����DACֵ
	}
	else
	{
		DAC_SetChannel1Data(DAC_Align_12b_R, 0xFFF);//12λ�Ҷ������ݸ�ʽ����DACֵ
	}
	
	return;
}

void Dac1_Sub_Val(u16 val)
{
	u16 currentAdcValue;
	currentAdcValue = DAC_GetDataOutputValue(DAC_Channel_1);
	
	if(currentAdcValue > val)
	{
		DAC_SetChannel1Data(DAC_Align_12b_R, currentAdcValue - val);//12λ�Ҷ������ݸ�ʽ����DACֵ
	}
	else
	{
		DAC_SetChannel1Data(DAC_Align_12b_R, 0);//12λ�Ҷ������ݸ�ʽ����DACֵ
	}
	
	return;
}


