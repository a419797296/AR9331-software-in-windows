#ifndef GPIO_EXAMPLE_
#define GPIO_EXAMPLE_
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/kfifo.h>
#include <linux/gpio.h>

#define AD7606_MAJOR 					98//device major number
#define AD7606_DEV_NAME		"inspection_ad7606"
#define CH_NUM			2				/* 采集1通道 */


#define AD7606_CV_AB			22
#define AD7606_CLK			18
#define AD7606_MISO			20
#define AD7606_RESET			21
#define AD7606_BUSY			19

#define AD7606DEV_SIZE			1024
#define AD7606_CHANNELS			8

#define AD7606_IOC_MAGIC	'B'
#define AD7606_IOC_SET_ACQUSITION_PARA	_IOW(AD7606_IOC_MAGIC,0,AD7606_ACQUSITION_PARA)
#define AD7606_IOC_SET_FREQ				_IOW(AD7606_IOC_MAGIC,1,int)
#define AD7606_IOC_SET_LENGTH			_IOW(AD7606_IOC_MAGIC,2,int)
#define AD7606_IOC_SET_CHANEL			_IOW(AD7606_IOC_MAGIC,3,int)
#define AD7606_IOC_START				_IO(AD7606_IOC_MAGIC,4)

// #define DEBUG				1//Byte
// #define PDEBUG				0//Byte
#ifdef PDEBUG
 #define PLOG(fmt, args...) printk( KERN_DEBUG "AD7606: " fmt, ## args)
 #else
 #define PLOG(fmt, args...)
 #endif

#define AD7606_IOC_MAXNR	4
static struct gpio ad7606_gpios[] = {
	{ AD7606_CV_AB, GPIOF_OUT_INIT_LOW, "start convert the ad7606" }, 
	{ AD7606_CLK, GPIOF_OUT_INIT_LOW,  "spi clock" },  
	{ AD7606_MISO, GPIOF_DIR_IN,  "spi data"   }, 
	{ AD7606_RESET, GPIOF_OUT_INIT_LOW,  "ad7606 reset"  },
	{ AD7606_BUSY,GPIOF_DIR_IN,"ad7606 busy signal"}
};

typedef struct{
	int num;
	char name[10];
	short int *data;
}AD7606_CHANNEL_INFO;

typedef struct{
	int 					freq;
	int 					length;	
	char 					valid_channel_nums;	
	char 					valid_channel_list[8];
}AD7606_ACQUSITION_PARA;

typedef struct{
	struct cdev				s_cdev;
	struct class			*ad7606_dev_class;
	dev_t 					devno;
	AD7606_ACQUSITION_PARA	acqusition_para;
	AD7606_CHANNEL_INFO		*channel_info;
}AD7606_DEVICE;
#endif