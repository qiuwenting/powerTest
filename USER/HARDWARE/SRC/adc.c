#include "global.h"



volatile u16 ADC_ConvertedValue[ADC_CHANNEL_COUNT];			//ADC采样的数据
volatile u16 ADCBufCount = 0;
volatile u16 ADC_Value[ADC_BUF_LEN];
volatile u8 fullFlag = 0;

volatile u16 ADCValueBuf1[ADCBUFSIZE];
volatile u16 ADCValueBuf2[ADCBUFSIZE];
volatile u8 readBuffer = 0;
volatile u8 writeBuffer = 1;

		 
void ADC_GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	  //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	  //使能GPIOC时钟
	
	//PA0 1 2 作为模拟通道输入引脚   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	//PC0 作为模拟通道输入引脚   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);  	
}
 
		   
//初始化ADC															   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure;
	
	ADC_TIM2_Init(9,7199);		//72000000/7200=10000Hz，10000/10 = 1000Hz 每0.001s采集一次
	DMA1_Init();
	ADC_GPIO_Init();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	  //使能ADC1通道时钟

	//ADC1初始化
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 			//独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;  			//关闭扫描方式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;			//关闭连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;   	//使用外部触发模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 			//采集数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 4; 			//要转换的通道数目
	ADC_Init(ADC1, &ADC_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);				//配置ADC时钟，为PCLK2的6分频，即12MHz
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_1Cycles5);		//配置ADC1通道6为1.5个采样周期 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_1Cycles5);		//配置ADC1通道6为1.5个采样周期 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_1Cycles5);		//配置ADC1通道6为1.5个采样周期 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 4, ADC_SampleTime_1Cycles5);		//配置ADC1通道6为1.5个采样周期
	//使能ADC、DMA
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);
 
	ADC_ResetCalibration(ADC1);				//复位校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));				//等待校准寄存器复位完成
 
	ADC_StartCalibration(ADC1);				//ADC校准
	while(ADC_GetCalibrationStatus(ADC1));				//等待校准完成
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);		//设置外部触发模式使能

}	

void VoltagePrint(void)
{
	u32 channel1Voltage, channel2Voltage, channel3Voltage, channel10Voltage, tempSum = 0, DacVal = 0, DacVol;
	u8 i;
	
	
	if(fullFlag == 1)
	{
		DacVal = DAC_GetDataOutputValue(DAC_Channel_1);
		DacVol = DacVal * 3300 / 4096;
		
		#ifdef DEBUG_BOARD
		DacVol = DacVol * 43 / 10;
		#endif
		
		
		for(i = 0; i < ADC_BUF_LEN; i += ADC_CHANNEL_COUNT)
		{
			tempSum = tempSum + ADC_Value[i];
		}
		channel1Voltage = tempSum * 3300 / ADC_ROUND / 4096;
		tempSum = 0;
		
		for(i = 1; i < ADC_BUF_LEN; i += ADC_CHANNEL_COUNT)
		{
			tempSum = tempSum + ADC_Value[i];
		}
		channel2Voltage = tempSum * 3300 / ADC_ROUND / 4096;
		tempSum = 0;
		
		for(i = 2; i < ADC_BUF_LEN; i += ADC_CHANNEL_COUNT)
		{
			tempSum = tempSum + ADC_Value[i];
		}
		channel3Voltage = tempSum * 3300 / ADC_ROUND / 4096;
		tempSum = 0;
		
		for(i = 3; i < ADC_BUF_LEN; i += ADC_CHANNEL_COUNT)
		{
			tempSum = tempSum + ADC_Value[i];
		}
		channel10Voltage = tempSum * 3300 / ADC_ROUND / 4096;
		tempSum = 0;
		
		printf("DacSetV:%d; DacV:%d; V1:%d; V2:%d; V3:%d\n", DacVol, channel10Voltage, channel1Voltage, channel2Voltage, channel3Voltage);
		fullFlag = 0;
	}
	
}

void VoltageIntervalPrint(void)
{
		const u32 interval = 1000;//ms
		static u32 startCount = 0;
		u32 channel1Voltage, channel2Voltage, channel3Voltage, channel10Voltage, DacVal = 0, DacVol;
		
	  if((getCurrentTimerCount() - startCount) >= interval)
		{
			DacVal = DAC_GetDataOutputValue(DAC_Channel_1);
			DacVol = DacVal * 3300 / 4096;
			
			#ifdef DEBUG_BOARD
			DacVol = DacVol * 43 / 10;
			#endif
			
			channel1Voltage = ADC_Value[0] * 3300 / ADC_ROUND / 4096;
			channel2Voltage = ADC_Value[1] * 3300 / ADC_ROUND / 4096;
			channel3Voltage = ADC_Value[2] * 3300 / ADC_ROUND / 4096;
			channel10Voltage = ADC_Value[3] * 3300 / ADC_ROUND / 4096;
				
			printf("DacSetV:%d; DacV:%d; V1:%d; V2:%d; V3:%d\n", DacVol, channel10Voltage, channel1Voltage, channel2Voltage, channel3Voltage);
			startCount = getCurrentTimerCount();
		}
			
		return;
}



u8 isAdcValid(u16 adcVal)
{
		u8 ret = 0;
		if(adcVal > 100 && adcVal < 4000)
		{
			ret = 1;
		}
		return ret;
}

void selectAdcChannelToWriteInDmairq(void)
{
	static u32 i = 0;
	if(writeBuffer == 1)
	{
			if(isAdcValid(ADC_ConvertedValue[0]))
			{
				ADCValueBuf1[i] = ADC_ConvertedValue[0];
			}
			else if(isAdcValid(ADC_ConvertedValue[1]))
			{
				ADCValueBuf1[i] = ADC_ConvertedValue[1];
			}
			else if(isAdcValid(ADC_ConvertedValue[2]))
			{
				ADCValueBuf1[i] = ADC_ConvertedValue[2];
			}
			i++;
			if(i >= ADCBUFSIZE)
			{
				i = 0;
				writeBuffer = 2;
				readBuffer = 1;
				printf("dma write1\n");
			}
	}
	else if(writeBuffer == 2)
	{
			if(isAdcValid(ADC_ConvertedValue[0]))
			{
				ADCValueBuf2[i] = ADC_ConvertedValue[0];
			}
			else if(isAdcValid(ADC_ConvertedValue[1]))
			{
				ADCValueBuf2[i] = ADC_ConvertedValue[1];
			}
			else if(isAdcValid(ADC_ConvertedValue[2]))
			{
				ADCValueBuf2[i] = ADC_ConvertedValue[2];
			}
			i++;
			if(i >= ADCBUFSIZE)
			{
				i = 0;
				writeBuffer = 1;
				readBuffer = 2;
				printf("dma write2\n");
			}
	}
	
}


