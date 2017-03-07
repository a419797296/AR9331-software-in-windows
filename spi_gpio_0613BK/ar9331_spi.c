#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "ar9331_spi.h"
#include "ad7606.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

//-------------------------------------spi 模式----------------
static void do_read(int fd, unsigned char * ad_result,int len)
{
	int		status;
	/* read at least 2 bytes, no more than 32 */
	if (len < 2)
		len = 2;
	else if (len > sizeof(ad_result))
		len = sizeof(ad_result);
	memset(ad_result, 0, len);

	status = read(fd, ad_result, len);
	if (status < 0) {
		perror("read");
		return;
	}
	if (status != len) {
		fprintf(stderr, "short read\n");
		return;
	}
}
//---------------------------读取spi参数--------------
static void dumpstat(const char *name, int fd)
{
	__u8	mode, lsb, bits;
	__u32	speed;

	if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0) {
		perror("SPI rd_mode");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb) < 0) {
		perror("SPI rd_lsb_fist");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {
		perror("SPI bits_per_word");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		perror("SPI max_speed_hz");
		return;
	}

	printf("%s: spi mode %d, %d bits %sper word, %d Hz max\n",
		name, mode, bits, lsb ? "(lsb first) " : "", speed);
}
//----------------------------------启动并读取AD转换-----------
void readAD(int freq, int sampleNum,unsigned char * ad_result)
{
	int fd,i;

	for (i = 0; i < sampleNum; i++)
	{
		ad7606_StartConv();	/* 给开始信号 */
		//while (AD_BUSY_STAT()==1);
		fd = open(SPI_DEV_PATH,O_RDWR);
		do_read(fd,ad_result+i+i,ADdata_SIZE);
		printf("%d, %d\n",ad_result[i+i],ad_result[i+i+1]);
		//dumpstat(path, fd);
		close(fd);
		usleep(1000000/freq);
	}
}

//----------------------------------启动并读取AD转换-----------
void reportAD(char *devName, int timeInterval,unsigned char * ad_result)
{
	int fd;

		ad7606_StartConv();	/* 给开始信号 */
		//while (AD_BUSY_STAT()==1);
		fd = open(SPI_DEV_PATH,O_RDWR);
		do_read(fd,ad_result,ADdata_SIZE);
		printf("每隔%d秒上报一次数据，本次数据内容为：%d, %d\n",timeInterval,ad_result[0],ad_result[1]);
		//dumpstat(path, fd);
		close(fd);
		sleep(timeInterval);
}