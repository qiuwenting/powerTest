#include "global.h"



#define FLASH_MAXSIZE 0x1E847FFF
#define PARA_NUM			13   //��һ�����в���ռ�ÿռ�


struct W2Q128PraStr
{
	u8  offsetCount;
	u8  flashFullFlag;
	u8  flashWriteStatus;
	u8  flashReadStatus;
	u32 offsetAddr;//�ڶ���������ʼ�������
	u32 startAddr;
	u32 fileLen;
	
};

struct W2Q128PraStr W2Q128Pra = {
	.offsetCount = 0,
	.flashFullFlag = 0,
	.flashWriteStatus = 0,
	.flashReadStatus = 0,
	.offsetAddr = 0,
	.startAddr = 0,
	.fileLen = 0,
};


u16 W25QXX_TYPE=W25Q128;	//Ĭ����W25Q128

//4KbytesΪһ��Sector
//16������Ϊ1��Block
//W25Q128
//����Ϊ16M�ֽ�,����128��Block,4096��Sector 
													 
//��ʼ��SPI FLASH��IO��
void W25QXX_Init(void)
{	
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ�� 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 ���� 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);
 
  W25QXX_CS=1;				//SPI FLASH��ѡ��
	SPI2_Init();		   	//��ʼ��SPI
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ
	W25QXX_TYPE=W25QXX_ReadID();//��ȡFLASH ID.  
	
	readFilePara();
	
	

}  

//��ȡW25QXX��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 W25QXX_ReadSR(void)   
{  
	u8 byte=0;   
	W25QXX_CS=0;                            //ʹ������   
	SPI2_ReadWriteByte(W25X_ReadStatusReg); //���Ͷ�ȡ״̬�Ĵ�������    
	byte=SPI2_ReadWriteByte(0Xff);          //��ȡһ���ֽ�  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     
	return byte;   
} 
//дW25QXX״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void W25QXX_Write_SR(u8 sr)   
{   
	W25QXX_CS=0;                            //ʹ������   
	SPI2_ReadWriteByte(W25X_WriteStatusReg);//����дȡ״̬�Ĵ�������    
	SPI2_ReadWriteByte(sr);               	//д��һ���ֽ�  
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
}   
//W25QXXдʹ��	
//��WEL��λ   
void W25QXX_Write_Enable(void)   
{
	W25QXX_CS=0;                          	//ʹ������   
  SPI2_ReadWriteByte(W25X_WriteEnable); 	//����дʹ��  
	W25QXX_CS=1;                           	//ȡ��Ƭѡ     	      
} 
//W25QXXд��ֹ	
//��WEL����  
void W25QXX_Write_Disable(void)   
{  
	W25QXX_CS=0;                            //ʹ������   
  SPI2_ReadWriteByte(W25X_WriteDisable);  //����д��ָֹ��    
	W25QXX_CS=1;                            //ȡ��Ƭѡ     	      
} 		
//��ȡоƬID
//����ֵ����:				   
//0XEF13,��ʾоƬ�ͺ�ΪW25Q80  
//0XEF14,��ʾоƬ�ͺ�ΪW25Q16    
//0XEF15,��ʾоƬ�ͺ�ΪW25Q32  
//0XEF16,��ʾоƬ�ͺ�ΪW25Q64 
//0XEF17,��ʾоƬ�ͺ�ΪW25Q128 	  
u16 W25QXX_ReadID(void)
{
	u16 Temp = 0;	  
	W25QXX_CS=0;				    
	SPI2_ReadWriteByte(0x90);//���Ͷ�ȡID����	    
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	 			   
	Temp|=SPI2_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI2_ReadWriteByte(0xFF);	 
	W25QXX_CS=1;				    
	return Temp;
}   		    
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	W25QXX_CS=0;                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_ReadData);         	//���Ͷ�ȡ����   
    SPI2_ReadWriteByte((u8)((ReadAddr)>>16));  	//����24bit��ַ    
    SPI2_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI2_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI2_ReadWriteByte(0XFF);   	//ѭ������  
    }
	W25QXX_CS=1;  				    	      
}  
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    W25QXX_Write_Enable();                  	//SET WEL 
	W25QXX_CS=0;                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_PageProgram);      	//����дҳ����   
    SPI2_ReadWriteByte((u8)((WriteAddr)>>16)); 	//����24bit��ַ    
    SPI2_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI2_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI2_ReadWriteByte(pBuffer[i]);//ѭ��д��  
	W25QXX_CS=1;                            	//ȡ��Ƭѡ 
	W25QXX_Wait_Busy();					   		//�ȴ�д�����
} 
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//д�������
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
			else pageremain=NumByteToWrite; 	  //����256���ֽ���
		}
	};	    
} 
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535)   
u8 W25QXX_BUFFER[4096];		 
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
  W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//������ַ  
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
	while(1) 
	{	
		W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			W25QXX_Erase_Sector(secpos);		//�����������
			for(i=0;i<secremain;i++)	   		//����
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//д����������  

		}else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  				//ָ��ƫ��
			WriteAddr+=secremain;				//д��ַƫ��	   
		   	NumByteToWrite-=secremain;			//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;//��һ����������д����
			else secremain=NumByteToWrite;		//��һ����������д����
		}	 
	};	 
}

//��������оƬ		  
//�ȴ�ʱ�䳬��...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                 	 	//SET WEL 
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_ChipErase);        	//����Ƭ��������  
	W25QXX_CS=1;                            	//ȡ��Ƭѡ     	      
	W25QXX_Wait_Busy();   				   		//�ȴ�оƬ��������
}   
//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ��ɽ��������ʱ��:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
	//����falsh�������,������   
 	//printf("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  	//SET WEL 	 
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_SectorErase);      	//������������ָ�� 
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>16));  	//����24bit��ַ    
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI2_ReadWriteByte((u8)Dst_Addr);  
		W25QXX_CS=1;                            	//ȡ��Ƭѡ     	      
    W25QXX_Wait_Busy();   				   		//�ȴ��������
}  
//�ȴ�����
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);  		// �ȴ�BUSYλ���
}  
//�������ģʽ
void W25QXX_PowerDown(void)   
{ 
  	W25QXX_CS=0;                           	 	//ʹ������   
    SPI2_ReadWriteByte(W25X_PowerDown);        //���͵�������  
		W25QXX_CS=1;                            	//ȡ��Ƭѡ     	      
    delay_us(3);                               //�ȴ�TPD  
}   
//����
void W25QXX_WAKEUP(void)   
{  
  	W25QXX_CS=0;                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
		W25QXX_CS=1;                            	//ȡ��Ƭѡ     	      
    delay_us(3);                            	//�ȴ�TRES1
}   

void W25qxxCyclicWriteDataFlow(u16 *dataFlow, u32 dataLen)
{
	
	u32 tempAddr = 0;
	u8 databuf[ADCBUFSIZE*2];
	
	u32 i;
	
//	if(!W2Q128Pra.offsetAddr)//�տ���
//	{
//		W25QXX_Read(databuf, 0, PARA_NUM);
//		printf("read offset 0x%02x%02x%02x%02x\n\n", databuf[0], databuf[1], databuf[2], databuf[3]);
//		tempAddr = databuf[0];
//		W2Q128Pra.offsetAddr |= (tempAddr << 24) & 0xFF000000;
//		tempAddr = databuf[1];
//		W2Q128Pra.offsetAddr |= (tempAddr << 16) & 0x00FF0000;
//		tempAddr = databuf[2];
//		W2Q128Pra.offsetAddr |= (tempAddr << 8)  & 0x0000FF00;
//		tempAddr = databuf[3];
//		W2Q128Pra.offsetAddr |= tempAddr & 0x000000FF;
//		
//		printf("W2Q128Pra.offsetAddr:%x\n", W2Q128Pra.offsetAddr);
//	}
	
	for(i = 0; i < dataLen; i++)
	{
		databuf[i*2] = (dataFlow[i] >> 16) & 0xFF;
		databuf[i*2 + 1] = dataFlow[i] & 0xFF;	
	}
	
	//printf("1 startAddr:0x%x, first:0x%02x%02x, last:0x%02x%02x\n", W2Q128Pra.offsetAddr, databuf[0], databuf[1], databuf[dataLen*2 - 2], databuf[dataLen*2 - 1]);
	printf("1 offsAddr:%x, dataLen:%x\n", W2Q128Pra.offsetAddr, dataLen*2);
	if((W2Q128Pra.offsetAddr += dataLen*2) > FLASH_MAXSIZE)
	{
		//��д������ݳ���flash������ַ����ַ��ת���ڶ�����ѭ��д��
		W25QXX_Write(databuf, W2Q128Pra.offsetAddr, FLASH_MAXSIZE - W2Q128Pra.offsetAddr + 1);//�ӵ�ǰ��ַд��flash��β
		W2Q128Pra.offsetAddr = 0x1000;//offset��ַ��ת���ڶ�����
		W25QXX_Write(databuf, W2Q128Pra.offsetAddr, dataLen*2 - (FLASH_MAXSIZE - W2Q128Pra.offsetAddr + 1));//�ӵڶ�������ʼ��ʣ�������д��
		//read for debug
		//W25QXX_Read(databuf, W2Q128Pra.offsetAddr, dataLen*2);
		//printf("2 startAddr:0x%d, first:0x%02x%02x, last:0x%02x%02x\n", W2Q128Pra.offsetAddr, databuf[0], databuf[1], databuf[dataLen*2 - 2], databuf[dataLen*2 - 1]);
		
		W2Q128Pra.offsetAddr = 0x1000 + dataLen*2 - (FLASH_MAXSIZE - W2Q128Pra.offsetAddr + 1);
		printf("2 offsAddr:%x, dataLen:%x\n", W2Q128Pra.offsetAddr, dataLen*2);
	}
	else
	{
		W25QXX_Write(databuf, W2Q128Pra.offsetAddr, dataLen*2);
		W2Q128Pra.offsetAddr += dataLen*2;
		//printf("2 startAddr:0x%x, first:0x%02x%02x, last:0x%02x%02x\n", W2Q128Pra.offsetAddr, databuf[0], databuf[1], databuf[dataLen*2 - 2], databuf[dataLen*2 - 1]);
		//W2Q128Pra.offsetAddr = 0x1000 + dataLen*2 - (FLASH_MAXSIZE - W2Q128Pra.offsetAddr + 1);
		printf("3 offsAddr:%x, dataLen:%x\n", W2Q128Pra.offsetAddr, dataLen*2);
	}	
	W2Q128Pra.fileLen += dataLen*2;//�ļ���������
	if(W2Q128Pra.fileLen >= FLASH_MAXSIZE)
	{
		W2Q128Pra.fileLen = FLASH_MAXSIZE;//flash�Ѿ�װ��
		W2Q128Pra.flashFullFlag = 1;
	}
	
	W2Q128Pra.offsetCount++;
	if(W2Q128Pra.offsetCount >= 10)
	{
		databuf[0] = (W2Q128Pra.startAddr >> 24);
		databuf[1] = (W2Q128Pra.startAddr >> 16);
		databuf[2] = (W2Q128Pra.startAddr >> 8);
		databuf[3] = W2Q128Pra.startAddr;
		
		databuf[4] = (W2Q128Pra.offsetAddr >> 24);
		databuf[5] = (W2Q128Pra.offsetAddr >> 16);
		databuf[6] = (W2Q128Pra.offsetAddr >> 8);
		databuf[7] = W2Q128Pra.offsetAddr;
		
		databuf[8] = (W2Q128Pra.fileLen >> 24);
		databuf[9] = (W2Q128Pra.fileLen >> 16);
		databuf[10] = (W2Q128Pra.fileLen >> 8);
		databuf[11] = W2Q128Pra.fileLen;
		
		databuf[12] = W2Q128Pra.flashFullFlag;
		printf("write offset 0x%02x%02x%02x%02x%02x%02x%02x%02x\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7]);
		W25QXX_Write(databuf, 0, PARA_NUM);//��һ�������洢������Ϣ
		//read for debug
		W25QXX_Read(databuf, 0, PARA_NUM);
		printf("read offset 0x%02x%02x%02x%02x\n\n", databuf[0], databuf[1], databuf[2], databuf[3]);
		W2Q128Pra.offsetCount = 0;
	}
	return;
}

//����ѭ�����еķ�ʽ��flash������һ�κ�beginAddr���ر��ֶ�ȡ���ݵ���һ�� ��ַ
void W25qxxCyclicReadData(u16* dataBuf2B, u32* beginAddr, u32 dataLen)
{
	u32 i;
	u16 tempval;
	u8 databuf[ADCBUFSIZE*2];
	u32 nextAddr = 0;
	if(dataLen > ADCBUFSIZE)
	{
		printf("read too many at one time\n");
		return;
	}
	
	if((*beginAddr + dataLen) > FLASH_MAXSIZE)
	{
		W25QXX_Read(databuf, *beginAddr, FLASH_MAXSIZE - *beginAddr + 1);
		W25QXX_Read(&databuf[FLASH_MAXSIZE - *beginAddr + 1], 0x1000, dataLen*2 - (FLASH_MAXSIZE - *beginAddr + 1));
		nextAddr = 0x1000 + dataLen*2 - (FLASH_MAXSIZE - *beginAddr + 1);
	}
	else
	{
		W25QXX_Read(databuf, *beginAddr, dataLen*2);
		nextAddr = *beginAddr + dataLen*2;
	}
	
	for(i = 0; i < ADCBUFSIZE; i++)
	{
		tempval = databuf[i*2];
		dataBuf2B[i] = tempval << 8 | databuf[i*2 + 1];
	}
	*beginAddr = nextAddr;
	return;
	
}

void readFilePara(void)
{
	  u32 tempAddr = 0;
	  u8 databuf[ADCBUFSIZE];
		W25QXX_Read(databuf, 0, PARA_NUM);
		printf("read offset 0x%02x%02x%02x%02x\n\n", databuf[0], databuf[1], databuf[2], databuf[3]);
		
	  //�����ļ���ʼ��ַ
		tempAddr = databuf[0];
		W2Q128Pra.startAddr |= (tempAddr << 24) & 0xFF000000;
		tempAddr = databuf[1];
		W2Q128Pra.startAddr |= (tempAddr << 16) & 0x00FF0000;
		tempAddr = databuf[2];
		W2Q128Pra.startAddr |= (tempAddr << 8)  & 0x0000FF00;
		tempAddr = databuf[3];
		W2Q128Pra.startAddr |= tempAddr & 0x000000FF;
	
		//�����ļ�������ַ
		tempAddr = databuf[4];
		W2Q128Pra.offsetAddr |= (tempAddr << 24) & 0xFF000000;
		tempAddr = databuf[5];
		W2Q128Pra.offsetAddr |= (tempAddr << 16) & 0x00FF0000;
		tempAddr = databuf[6];
		W2Q128Pra.offsetAddr |= (tempAddr << 8)  & 0x0000FF00;
		tempAddr = databuf[7];
		W2Q128Pra.offsetAddr |= tempAddr & 0x000000FF;
		
		//�����ļ�����
		tempAddr = databuf[8];
		W2Q128Pra.fileLen |= (tempAddr << 24) & 0xFF000000;
		tempAddr = databuf[9];
		W2Q128Pra.fileLen |= (tempAddr << 16) & 0x00FF0000;
		tempAddr = databuf[10];
		W2Q128Pra.fileLen |= (tempAddr << 8)  & 0x0000FF00;
		tempAddr = databuf[11];
		W2Q128Pra.fileLen |= tempAddr & 0x000000FF;
		
		//flash �Ƿ�����־
		tempAddr = databuf[12];
		W2Q128Pra.flashFullFlag = tempAddr & 0x000000FF;
	
}

void resetFile(void)
{
	u8 databuf[ADCBUFSIZE];
	
	if(W2Q128Pra.offsetAddr == 0)
	{
		W2Q128Pra.offsetAddr = 0x1000;
	}
	
	W2Q128Pra.startAddr = W2Q128Pra.offsetAddr;
	databuf[0] = (W2Q128Pra.startAddr >> 24);
	databuf[1] = (W2Q128Pra.startAddr >> 16);
	databuf[2] = (W2Q128Pra.startAddr >> 8);
	databuf[3] = W2Q128Pra.startAddr;
	
	databuf[4] = (W2Q128Pra.offsetAddr >> 24);
	databuf[5] = (W2Q128Pra.offsetAddr >> 16);
	databuf[6] = (W2Q128Pra.offsetAddr >> 8);
	databuf[7] = W2Q128Pra.offsetAddr;
	
	W2Q128Pra.fileLen = 0;
	databuf[8] = (W2Q128Pra.fileLen >> 24);
	databuf[9] = (W2Q128Pra.fileLen >> 16);
	databuf[10] = (W2Q128Pra.fileLen >> 8);
	databuf[11] = W2Q128Pra.fileLen;
	
	W2Q128Pra.flashFullFlag = 0;
	databuf[12] = W2Q128Pra.flashFullFlag;
	printf("write offset 0x%02x%02x%02x%02x%02x%02x%02x%02x\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7]);
	W25QXX_Write(databuf, 0, PARA_NUM);//��һ�������洢������Ϣ
		//read for debug
	//W25QXX_Read(databuf, 0, PARA_NUM);
	//printf("read offset 0x%02x%02x%02x%02x%02x%02x%02x%02x\n\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7]);
}


void writeFile(void)//����ADC���ݵ�˫����������дflash
{
	if(W2Q128Pra.flashWriteStatus && !W2Q128Pra.flashReadStatus)
	{
		if(readBuffer == 1)
		{
			printf("read buffer1\n");
			W25qxxCyclicWriteDataFlow((u16*)ADCValueBuf1, ADCBUFSIZE);	
			readBuffer = 0;
		}
		else if(readBuffer == 2)
		{
			printf("read buffer2\n");
			W25qxxCyclicWriteDataFlow((u16*)ADCValueBuf2, ADCBUFSIZE);
			readBuffer = 0;
		}
	}
	return;	
}

void readFile(void)
{
	const u16  readLen = 512;
	static u8  startFlag = 1;
	static u32 startAddr = 0;
	static u32 fileLen   = 0;
	u16 dataBuf2B[ADCBUFSIZE];
	if(W2Q128Pra.flashReadStatus && !W2Q128Pra.flashWriteStatus)
	{
		if(startFlag)
		{
			startAddr = W2Q128Pra.startAddr;
			fileLen = W2Q128Pra.fileLen;
			startFlag = 0;
		}
		if(readLen < fileLen)
		{
			W25qxxCyclicReadData(dataBuf2B, &startAddr, readLen);//����һ�κ�startAddr��ַ�Զ���ת
			fileLen -= readLen;
			printf("%d", startAddr);
		}
		else
		{
			W25qxxCyclicReadData(dataBuf2B, &startAddr, fileLen);//����һ�κ�startAddr��ַ�Զ���ת
			fileLen = 0;
			printf("finish %d", startAddr);
			W2Q128Pra.flashReadStatus = 0;
		}
	}
}

void continueLastWrite(void)
{
	readFilePara();
	W2Q128Pra.flashWriteStatus = 1;
}

void startNewWrite(void)
{
	resetFile();
	W2Q128Pra.flashWriteStatus = 1;
}

void stopFileWrite(void)
{
	W2Q128Pra.flashWriteStatus = 0;
}

void finishFileWrite(void)
{
	u8 databuf[ADCBUFSIZE];
	W2Q128Pra.flashWriteStatus = 0;
	databuf[0] = (W2Q128Pra.startAddr >> 24);
	databuf[1] = (W2Q128Pra.startAddr >> 16);
	databuf[2] = (W2Q128Pra.startAddr >> 8);
	databuf[3] = W2Q128Pra.startAddr;
		
	databuf[4] = (W2Q128Pra.offsetAddr >> 24);
	databuf[5] = (W2Q128Pra.offsetAddr >> 16);
	databuf[6] = (W2Q128Pra.offsetAddr >> 8);
	databuf[7] = W2Q128Pra.offsetAddr;
	
	databuf[8] = (W2Q128Pra.fileLen >> 24);
	databuf[9] = (W2Q128Pra.fileLen >> 16);
	databuf[10] = (W2Q128Pra.fileLen >> 8);
	databuf[11] = W2Q128Pra.fileLen;
		
	databuf[12] = W2Q128Pra.flashFullFlag;
	printf("write offset 0x%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7], databuf[8], databuf[9], databuf[10], databuf[11], databuf[12]);
	W25QXX_Write(databuf, 0, PARA_NUM);//��һ�������洢������Ϣ
	//read for debug
	W25QXX_Read(databuf, 0, PARA_NUM);
	printf("read  offset 0x%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7], databuf[8], databuf[9], databuf[10], databuf[11], databuf[12]);	
	return;
}

void startReadFile(void)
{
	W2Q128Pra.flashReadStatus = 1;
}


















