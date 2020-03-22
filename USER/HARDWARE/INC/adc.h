#ifndef __ADC_H
#define __ADC_H

#include "global.h"

#define ADC1_DR_Address    ((u32)0x4001244C)		//ADC1µÄµØÖ·
#define ADC_CHANNEL_COUNT 4
#define ADC_ROUND         10
#define ADC_BUF_LEN ADC_ROUND*ADC_CHANNEL_COUNT

#define ADCBUFSIZE	1024




extern volatile u16 ADC_ConvertedValue[];	
extern volatile u16 ADCBufCount;
extern volatile u16 ADC_Value[];
extern volatile u8 fullFlag;

extern volatile u8 readBuffer;

extern volatile u16 ADCValueBuf1[];
extern volatile u16 ADCValueBuf2[];


void Adc_Init(void);
u16  Get_Adc(u8 ch); 
u16 Get_Adc_Average(u8 ch,u8 times); 

extern void VoltagePrint(void);
extern void VoltageIntervalPrint(void);
extern void selectAdcChannelToWriteInDmairq(void);
#endif 
