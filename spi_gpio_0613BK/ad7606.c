#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gpio.h"
#include "ad7606.h"
#include "ar9331_spi.h"

short int s_adc_now[8];
int numList[5]={SPI_CV_AB,SPI_CLK,AD7606_RESET,AD7606_BUSY,SPI_MISO};

void ad7606_Init(char *mode)
{	
	if (strcmp(mode,"gpio")==0)
	{
		if (isFileExist(SPI_DEV_PATH)==0)   //如果存在SPI驱动文件
		{
			char * realiseSpiDevCmd="rmmod spi-gpio-custom";
			system(realiseSpiDevCmd);
			// CLK_FILE=gpioInit(SPI_CLK,"out");
			// MISO_FILE=gpioInit(SPI_MISO,"in");
			// BUSY_FILE=gpioInit(AD7606_BUSY,"in");

			gpioInit(SPI_CLK,"out");
			gpioInit(SPI_MISO,"in");
			gpioInit(AD7606_BUSY,"in");
		}
	}
	else if (strcmp(mode,"spi")==0)
	{
		if (isFileExist(SPI_DEV_PATH)==-1)
		{
			char * creatSpiDevCmd="insmod spi-gpio-custom bus0=1,18,100,20,2,15000000";
			gpioRelease(numList);
			// printf("%s\n", creatSpiDevCmd);
			system(creatSpiDevCmd);
		}
	}
	else
	{
		printf("can't identify the spi drive mode\n");;
	}
	// CV_AB_FILE=gpioInit(SPI_CV_AB,"out");   //ad7606开始转换信号初始化
	gpioInit(SPI_CV_AB,"out");   //ad7606开始转换信号初始化
	gpioInit(AD7606_RESET,"out");			//ad7606复位信号初始化
	ad7606_Reset();   //复位
	AD_CONVST_HIGH();   //转换初始化为高电平，其上升沿有效
	printf("-------------------initial finished------------------\n");
}

//-------------------------------------------
void ad7606_Reset(void)
{
	AD_RESET_LOW();
	usleep(10);
	AD_RESET_HIGH();
	usleep(10);
	AD_RESET_LOW();
	usleep(10);
}

//--------------------------------------------
void ad7606_StartConv(void)
{
	/* 上升沿开始转换，低电平持续时间至少25ns  */
	AD_CONVST_LOW();
	usleep(1);	
	AD_CONVST_HIGH();   
}

//--------------------------------------------
short int ad7606_ReadBytes(void)
{
	short int usData = 0;
	int i;

	/* spi */	
	for (i = 0; i < 16; i++)
	{
		AD_CLK_LOW();
		usData = usData << 1;
		usData |= AD_MISO_STAT();
		AD_CLK_HIGH();
	}
	// printf("the AD value is: %d\n", usData);
	//usData=(long)usData*5000 / 32767;
	// printf("the volt is: %d\n", usData);

	return usData;		
}
//-------------------------------------------------
void ad7606_Scan(void) 		/* 此函数代码按照时序编写 */
{
	int i;
	/* BUSY = 0 时.ad7606处于空闲状态ad转换结束 */
	// printf("the AD7606_BUSY is: %d\n", AD_BUSY_STAT());	
	if(AD_BUSY_STAT()==0)   
    {
	//	AD_CS_LOW(); /* SPI片选 = 0 */
		for (i = 0; i < CH_NUM; i++)
		{
			s_adc_now[i] = ad7606_ReadBytes(); /* 读数据 */	
			s_adc_now[i]=s_adc_now[i]*5000 / 32767;
			printf("%8d\n", s_adc_now[i] );
		}
	
//		AD_CS_HIGH(); /* SPI片选 = 1 */
		ad7606_StartConv();	/* 给开始信号 */		 	
		while (AD_BUSY_STAT()==1);
	}
}	  

int isFileExist(char * path)
{
	if (access(path, F_OK) == -1) { 
       puts("File not exists!"); 
       return(-1); 
   	} 
   	if (access(path, R_OK) == -1) 
    {
       	puts("You can't read the file!"); 
   		return(-1); 
    }

   	else 
   	{
   		if (access(path, R_OK | W_OK) != -1) 
           puts("You can read and write the file"); 
       	else 
           puts("You can read the file"); 
        return 0;
   	}
}

