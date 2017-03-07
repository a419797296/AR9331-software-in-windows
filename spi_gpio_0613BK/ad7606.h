#ifndef AD7606_H_
#define AD7606_H_

#define CH_NUM			1				/* 采集1通道 */
#define ADdata_SIZE     2

#define AD7606_RESET			21
#define AD7606_BUSY			19


#define AD_CONVST_LOW()					gpioSetValue(SPI_CV_AB,'0')
#define AD_CONVST_HIGH()				gpioSetValue(SPI_CV_AB,'1')

#define AD_CLK_LOW()					gpioSetValue(SPI_CLK,'0')
#define AD_CLK_HIGH()					gpioSetValue(SPI_CLK,'1')

#define AD_RESET_LOW()					gpioSetValue(AD7606_RESET,'0')
#define AD_RESET_HIGH()					gpioSetValue(AD7606_RESET,'1')

#define AD_BUSY_STAT()					gpioGetValue(AD7606_BUSY)

#define AD_MISO_STAT()					gpioGetValue(SPI_MISO)

void ad7606_Init(char *mode);
void ad7606_Reset(void);
void ad7606_StartConv(void);
void ad7606_Scan(void);
short int ad7606_ReadBytes(void);
int isFileExist(char * path);


#endif