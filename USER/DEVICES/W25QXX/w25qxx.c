#include "global.h"



#define FLASH_MAXSIZE 0x1E847FFF
#define PARA_NUM			13   //第一扇区中参数占用空间


struct W2Q128PraStr
{
	u8  offsetCount;
	u8  flashFullFlag;
	u8  flashWriteStatus;
	u8  flashReadStatus;
	u32 offsetAddr;//第二个扇区开始存放数据
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


u16 W25QXX_TYPE=W25Q128;	//默认是W25Q128

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q128
//容量为16M字节,共有128个Block,4096个Sector 
													 
//初始化SPI FLASH的IO口
void W25QXX_Init(void)
{	
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 推挽 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);
 
  W25QXX_CS=1;				//SPI FLASH不选中
	SPI2_Init();		   	//初始化SPI
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式
	W25QXX_TYPE=W25QXX_ReadID();//读取FLASH ID.  
	
	readFilePara();
	
	

}  

//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 W25QXX_ReadSR(void)   
{  
	u8 byte=0;   
	W25QXX_CS=0;                            //使能器件   
	SPI2_ReadWriteByte(W25X_ReadStatusReg); //发送读取状态寄存器命令    
	byte=SPI2_ReadWriteByte(0Xff);          //读取一个字节  
	W25QXX_CS=1;                            //取消片选     
	return byte;   
} 
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(u8 sr)   
{   
	W25QXX_CS=0;                            //使能器件   
	SPI2_ReadWriteByte(W25X_WriteStatusReg);//发送写取状态寄存器命令    
	SPI2_ReadWriteByte(sr);               	//写入一个字节  
	W25QXX_CS=1;                            //取消片选     	      
}   
//W25QXX写使能	
//将WEL置位   
void W25QXX_Write_Enable(void)   
{
	W25QXX_CS=0;                          	//使能器件   
  SPI2_ReadWriteByte(W25X_WriteEnable); 	//发送写使能  
	W25QXX_CS=1;                           	//取消片选     	      
} 
//W25QXX写禁止	
//将WEL清零  
void W25QXX_Write_Disable(void)   
{  
	W25QXX_CS=0;                            //使能器件   
  SPI2_ReadWriteByte(W25X_WriteDisable);  //发送写禁止指令    
	W25QXX_CS=1;                            //取消片选     	      
} 		
//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
u16 W25QXX_ReadID(void)
{
	u16 Temp = 0;	  
	W25QXX_CS=0;				    
	SPI2_ReadWriteByte(0x90);//发送读取ID命令	    
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	 			   
	Temp|=SPI2_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI2_ReadWriteByte(0xFF);	 
	W25QXX_CS=1;				    
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	W25QXX_CS=0;                            	//使能器件   
    SPI2_ReadWriteByte(W25X_ReadData);         	//发送读取命令   
    SPI2_ReadWriteByte((u8)((ReadAddr)>>16));  	//发送24bit地址    
    SPI2_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI2_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI2_ReadWriteByte(0XFF);   	//循环读数  
    }
	W25QXX_CS=1;  				    	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    W25QXX_Write_Enable();                  	//SET WEL 
	W25QXX_CS=0;                            	//使能器件   
    SPI2_ReadWriteByte(W25X_PageProgram);      	//发送写页命令   
    SPI2_ReadWriteByte((u8)((WriteAddr)>>16)); 	//发送24bit地址    
    SPI2_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI2_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI2_ReadWriteByte(pBuffer[i]);//循环写数  
	W25QXX_CS=1;                            	//取消片选 
	W25QXX_Wait_Busy();					   		//等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
u8 W25QXX_BUFFER[4096];		 
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
  W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25QXX_Erase_Sector(secpos);		//擦除这个扇区
			for(i=0;i<secremain;i++)	   		//复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  

		}else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  				//指针偏移
			WriteAddr+=secremain;				//写地址偏移	   
		   	NumByteToWrite-=secremain;			//字节数递减
			if(NumByteToWrite>4096)secremain=4096;//下一个扇区还是写不完
			else secremain=NumByteToWrite;		//下一个扇区可以写完了
		}	 
	};	 
}

//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                 	 	//SET WEL 
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            	//使能器件   
    SPI2_ReadWriteByte(W25X_ChipErase);        	//发送片擦除命令  
	W25QXX_CS=1;                            	//取消片选     	      
	W25QXX_Wait_Busy();   				   		//等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
	//监视falsh擦除情况,测试用   
 	//printf("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  	//SET WEL 	 
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            	//使能器件   
    SPI2_ReadWriteByte(W25X_SectorErase);      	//发送扇区擦除指令 
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>16));  	//发送24bit地址    
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI2_ReadWriteByte((u8)Dst_Addr);  
		W25QXX_CS=1;                            	//取消片选     	      
    W25QXX_Wait_Busy();   				   		//等待擦除完成
}  
//等待空闲
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);  		// 等待BUSY位清空
}  
//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
  	W25QXX_CS=0;                           	 	//使能器件   
    SPI2_ReadWriteByte(W25X_PowerDown);        //发送掉电命令  
		W25QXX_CS=1;                            	//取消片选     	      
    delay_us(3);                               //等待TPD  
}   
//唤醒
void W25QXX_WAKEUP(void)   
{  
  	W25QXX_CS=0;                            	//使能器件   
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
		W25QXX_CS=1;                            	//取消片选     	      
    delay_us(3);                            	//等待TRES1
}   

void W25qxxCyclicWriteDataFlow(u16 *dataFlow, u32 dataLen)
{
	
	u32 tempAddr = 0;
	u8 databuf[ADCBUFSIZE*2];
	
	u32 i;
	
//	if(!W2Q128Pra.offsetAddr)//刚开机
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
		//新写入的数据超过flash的最大地址，地址跳转到第二扇区循环写入
		W25QXX_Write(databuf, W2Q128Pra.offsetAddr, FLASH_MAXSIZE - W2Q128Pra.offsetAddr + 1);//从当前地址写到flash结尾
		W2Q128Pra.offsetAddr = 0x1000;//offset地址跳转到第二扇区
		W25QXX_Write(databuf, W2Q128Pra.offsetAddr, dataLen*2 - (FLASH_MAXSIZE - W2Q128Pra.offsetAddr + 1));//从第二扇区开始将剩余的数据写完
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
	W2Q128Pra.fileLen += dataLen*2;//文件长度增加
	if(W2Q128Pra.fileLen >= FLASH_MAXSIZE)
	{
		W2Q128Pra.fileLen = FLASH_MAXSIZE;//flash已经装满
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
		W25QXX_Write(databuf, 0, PARA_NUM);//第一个扇区存储基本信息
		//read for debug
		W25QXX_Read(databuf, 0, PARA_NUM);
		printf("read offset 0x%02x%02x%02x%02x\n\n", databuf[0], databuf[1], databuf[2], databuf[3]);
		W2Q128Pra.offsetCount = 0;
	}
	return;
}

//按照循环队列的方式读flash，读完一次后，beginAddr返回本轮读取数据的下一个 地址
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
		
	  //储存文件起始地址
		tempAddr = databuf[0];
		W2Q128Pra.startAddr |= (tempAddr << 24) & 0xFF000000;
		tempAddr = databuf[1];
		W2Q128Pra.startAddr |= (tempAddr << 16) & 0x00FF0000;
		tempAddr = databuf[2];
		W2Q128Pra.startAddr |= (tempAddr << 8)  & 0x0000FF00;
		tempAddr = databuf[3];
		W2Q128Pra.startAddr |= tempAddr & 0x000000FF;
	
		//储存文件结束地址
		tempAddr = databuf[4];
		W2Q128Pra.offsetAddr |= (tempAddr << 24) & 0xFF000000;
		tempAddr = databuf[5];
		W2Q128Pra.offsetAddr |= (tempAddr << 16) & 0x00FF0000;
		tempAddr = databuf[6];
		W2Q128Pra.offsetAddr |= (tempAddr << 8)  & 0x0000FF00;
		tempAddr = databuf[7];
		W2Q128Pra.offsetAddr |= tempAddr & 0x000000FF;
		
		//储存文件长度
		tempAddr = databuf[8];
		W2Q128Pra.fileLen |= (tempAddr << 24) & 0xFF000000;
		tempAddr = databuf[9];
		W2Q128Pra.fileLen |= (tempAddr << 16) & 0x00FF0000;
		tempAddr = databuf[10];
		W2Q128Pra.fileLen |= (tempAddr << 8)  & 0x0000FF00;
		tempAddr = databuf[11];
		W2Q128Pra.fileLen |= tempAddr & 0x000000FF;
		
		//flash 是否满标志
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
	W25QXX_Write(databuf, 0, PARA_NUM);//第一个扇区存储基本信息
		//read for debug
	//W25QXX_Read(databuf, 0, PARA_NUM);
	//printf("read offset 0x%02x%02x%02x%02x%02x%02x%02x%02x\n\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7]);
}


void writeFile(void)//利用ADC数据的双缓冲区轮流写flash
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
			W25qxxCyclicReadData(dataBuf2B, &startAddr, readLen);//读完一次后startAddr地址自动跳转
			fileLen -= readLen;
			printf("%d", startAddr);
		}
		else
		{
			W25qxxCyclicReadData(dataBuf2B, &startAddr, fileLen);//读完一次后startAddr地址自动跳转
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
	W25QXX_Write(databuf, 0, PARA_NUM);//第一个扇区存储基本信息
	//read for debug
	W25QXX_Read(databuf, 0, PARA_NUM);
	printf("read  offset 0x%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x\n", databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6], databuf[7], databuf[8], databuf[9], databuf[10], databuf[11], databuf[12]);	
	return;
}

void startReadFile(void)
{
	W2Q128Pra.flashReadStatus = 1;
}


















