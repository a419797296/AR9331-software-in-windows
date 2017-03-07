#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>  
#include <linux/types.h>
#include <linux/gpio.h>
#include <asm/delay.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include <linux/timer.h>
#include <linux/cdev.h>
#include <linux/fs.h>
// #include <linux/irq.h>
#include <linux/hrtimer.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/sched.h> 
#include <asm-generic/errno-base.h>

#include <linux/jiffies.h>

// #include <linux/kfifo.h>
// #include <asm-???/delay.h>
#include "ad7606.h"
static AD7606_DEVICE ad7606_device;
static struct hrtimer timer;
int cur_acqusition_length = 0;
ktime_t kt;

static DECLARE_WAIT_QUEUE_HEAD(ad7606_waitq);  
static volatile int finish_acquisition = 0;  

int ad7606_init(void);
void ad7606_Reset(void);
void ad7606_StartConv(void);
void ad7606_Scan(short int*s_adc_now); 		/* 此函数代码按照时序编写 */
short int ad7606_ReadBytes(void);
void ad7606_get_data(AD7606_ACQUSITION_PARA acqusition_para, AD7606_CHANNEL_INFO *channel_info);
static void ad7606_hrtimer_init(void);
static void ad7606_hrtimer_exit(void);
static enum hrtimer_restart ad7606_hrtimer_handler(struct hrtimer *timer);

int ad7606_init(void)
{
	int result;
	PLOG(KERN_INFO "ad7606 started\n");
	gpio_free_array(ad7606_gpios, ARRAY_SIZE(ad7606_gpios));
	result = gpio_request_array(ad7606_gpios, ARRAY_SIZE(ad7606_gpios));
	if (result < 0){
		PLOG("###gpio_request ERROR: can't request GPIOs###\n");
		return -1;
	}
	ad7606_Reset();
	return 0;
}
//------------------------------------------ad7606 module
//-------------------------------------------
void ad7606_Reset(void)
{
	gpio_set_value(AD7606_RESET, 0);
	udelay(10);
	gpio_set_value(AD7606_RESET, 0);
	udelay(10);
	gpio_set_value(AD7606_RESET, 0);
}

//--------------------------------------------
void ad7606_StartConv(void)
{
	/* 上升沿开始转换，低电平持续时间至少25ns  */
	gpio_set_value(AD7606_CV_AB, 0);
	udelay(2);
	gpio_set_value(AD7606_CV_AB, 1);
}

//--------------------------------------------
short int ad7606_ReadBytes(void)
{
	short int usData = 0;
	int i;

	/* spi */	
	for (i = 0; i < 16; i++)
	{
		gpio_set_value(AD7606_CLK, 0);
		usData = usData << 1;
		usData |= gpio_get_value(AD7606_MISO);
		gpio_set_value(AD7606_CLK, 1);
	}
	// PLOG("the AD value is: %d\n", usData);
	//usData=(long)usData*5000 / 32767;
	// printf("the volt is: %d\n", usData);

	return usData;		
}
//-------------------------------------------------
void ad7606_Scan(short int*s_adc_now) 		/* 此函数代码按照时序编写 */
{
	int i;
	/* BUSY = 0 时.ad7606处于空闲状态ad转换结束 */
	// printf("the AD7606_BUSY is: %d\n", AD_BUSY_STAT());	
	while (gpio_get_value(AD7606_BUSY)==1); //等待ad7606空闲
	if(gpio_get_value(AD7606_BUSY)==0)   
    {
	//	AD_CS_LOW(); /* SPI片选 = 0 */
		for (i = 0; i < AD7606_CHANNELS; i++)
		{
			s_adc_now[i] = ad7606_ReadBytes(); /* 读数据 */	
			s_adc_now[i]=(s_adc_now[i]*5000)>>15;
		}
	
		//AD_CS_HIGH(); /* SPI片选 = 1 */
		ad7606_StartConv();	/* 给开始信号 */		 	
		 
	}
}		
//-------------------------------------------------
void ad7606_get_data(AD7606_ACQUSITION_PARA acqusition_para, AD7606_CHANNEL_INFO *channel_info) 		/* 此函数代码按照时序编写 */
{
	int i,j,freq,length;
	char chl_nums, *chl_list;
	short int s_adc_now[8];

	freq=acqusition_para.freq;
	length=acqusition_para.length;
	chl_nums=acqusition_para.valid_channel_nums;
	chl_list=acqusition_para.valid_channel_list;
	ad7606_Scan(s_adc_now);  //清空之前寄存器保存的数据
	// if (length > AD7606DEV_SIZE)   //如果超过最大缓存，则采集最大缓存数
	// {
	// 	length = AD7606DEV_SIZE;
	// }
	//--------------------------------------------给通道号赋值
	// PLOG("chl_nums is : %d, freq is %d ---->\n", chl_nums, freq);
	// for (i = 0; i < chl_nums; i++)
	// {
	// 	(*(channel_info+i)).num = *(chl_list+i);
	// 	PLOG("*(chl_list+i) is : %d\n", *(chl_list+i));
	// }
		
	ad7606_hrtimer_init();    //开启高精度定时器
	// --------------------------------------------给通道的数据赋值

	// for (i = 0; i < length; i++)
	// {
	// 	ad7606_Scan(s_adc_now);
	// 	for (j = 0; j < chl_nums; ++j)
	// 		*((*(channel_info+j)).data+i) = s_adc_now[*(chl_list+j)];
	// 	// *(buf+i)=s_adc_now[chanle];
	// }

}


char gpio_num_list[5]={AD7606_CV_AB,AD7606_CLK,AD7606_RESET,AD7606_BUSY,AD7606_MISO};
// char *gpio_name_list[5]={AD7606_CV_AB_NAME,AD7606_CLK_NAME,AD7606_RESET_NAME,AD7606_BUSY_NAME,AD7606_MISO_NAME};
static int ad7606_open(struct inode *pinode, struct file *pfile)
{
	PLOG("***%s***\n",__func__);
	pfile->private_data = &ad7606_device;
	if(ad7606_init() < 0)
		return 0;
	return 0;
}

static loff_t ad7606_llseek(struct file *filp, loff_t offset, int orig)
{
	
	loff_t ret = 0;
	PLOG("***%s***\n",__func__);
	switch (orig) {
  case 0:
    if (offset < 0) {
      ret = -EINVAL;
      break;
    }
    if ((unsigned int)offset > AD7606DEV_SIZE) {
      ret = -EINVAL;
      break;
    }
    filp->f_pos = (unsigned int)offset;
    ret = filp->f_pos;
    break;
  case 1:
    if ((filp->f_pos + offset) > AD7606DEV_SIZE) {
      ret = -EINVAL;
      break;
    }
    if ((filp->f_pos + offset) < 0) {
      ret = -EINVAL;
      break;
    }
    filp->f_pos += offset;
    ret = filp->f_pos;
    break;
  default:
    ret = -EINVAL;
    break;
  }
  return ret;
}
static int ad7606_release(struct inode *pinode, struct file *pfile)
{
	PLOG("***%s***\n",__func__);

	return 0;
}

static ssize_t ad7606_read(struct file *pfile, char __user *pbuf, size_t size, loff_t *ppos)
{
	int i,j;
	char channel_nums, channel_list[8];
	int length, freq;
	short int s_adc_now[8];
	long int T;
	// unsigned long p = *ppos;
	unsigned long p = 0;  //从文件头开始读数据
	unsigned int count =size;
	int ret =0;
	PLOG("***kearnal: %s***\n",__func__);
	//-----------------------------------------从结构中获取参数
	channel_nums = ad7606_device.acqusition_para.valid_channel_nums;
	strcpy(channel_list, ad7606_device.acqusition_para.valid_channel_list);
	// printk("the channel_list is : %s\n", channel_list);
	length = ad7606_device.acqusition_para.length;
	freq = ad7606_device.acqusition_para.freq;

	ad7606_device.channel_info=(AD7606_CHANNEL_INFO *)kmalloc(count,GFP_KERNEL);
	//---------------------------------------------------指定内核空间数据地址及其对应的通道编号
	for (i = 0; i < channel_nums; ++i)
	{
		(*(ad7606_device.channel_info+i)).data=(short int *)(ad7606_device.channel_info + channel_nums) +length * i;
		(*(ad7606_device.channel_info+i)).num = *(channel_list+i) - '0';
		// printk("*(channel_list+i) is : %d\n", (*(ad7606_device.channel_info+i)).num);
	}
	
	ad7606_Scan(s_adc_now);  //清空之前寄存器保存的数据
	if(!freq)
	{
	// --------------------------------------------给通道的数据赋值
		for (i = 0; i < length; i++)
		{
			ad7606_Scan(s_adc_now);
			for (j = 0; j < channel_nums; ++j)
				*((*(ad7606_device.channel_info+j)).data+i) = s_adc_now[(*(ad7606_device.channel_info+j)).num];
			// *(buf+i)=s_adc_now[chanle];
		}
	}
	else
	{
		T = 1000000 / freq;    //单位为um
		kt = ktime_set(0, 1000 * T); /* 1 sec, 10 nsec */
		ad7606_hrtimer_init();    //开启高精度定时器
		// //---------------------------------------------------get the data
		// ad7606_get_data(ad7606_device.acqusition_para, ad7606_device.channel_info);
		if (!finish_acquisition)  //如果采集没有完成，让当前进程进入等待队列中
		{
			if (pfile->f_flags & O_NONBLOCK)  
			{
				PLOG("***---O_NONBLOCK--***\n");
	            return -EAGAIN;  
			}
	        else  
	            wait_event_interruptible(ad7606_waitq, finish_acquisition);
		}
		finish_acquisition = 0;
	}


#if DEBUG
	for (i = 0; i < channel_nums; ++i)
	{
		for (j = 0; j < ad7606_device.acqusition_para.length; ++j)
		{
			PLOG("%6d",*((*(ad7606_device.channel_info+i)).data+j) );
		}
		PLOG("\n");
	}
	PLOG("\nthe current file pos is %ld,count is %d \n", p,count);
#endif
	//---------------------------------------------------指定用户空间数据地址
	for (i = 0; i < channel_nums; ++i)
	{
		(*(ad7606_device.channel_info+i)).data=(short int *)((AD7606_CHANNEL_INFO *)pbuf + channel_nums) +length * i;
		// PLOG("---(*(channel_info+%d)).data is %d ---\n",i, (char *)((*(channel_info+i)).data)-(char *)pbuf);
	}

	//---------------------------------------------------copy the struct and data, data is follow the struct addr
	if (copy_to_user(pbuf , (void*)(ad7606_device.channel_info),count))
	{
		ret = - EFAULT;
		PLOG("----------------copy error-------------");
	}
	else
	{
		PLOG("read %d bytes(s) from %ld\n", count,p);
		p+=count;
	}
	ad7606_hrtimer_exit();
	kfree(ad7606_device.channel_info);
	return ret;
}

static ssize_t ad7606_write(struct file *pfile, const char __user *pbuf, size_t size, loff_t *ppos)
{
	PLOG("***%s***\n",__func__);

	return 0;
}

static long ad7606_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
	char *channel_list, channel_nums;
	int i;

	PLOG("***%s***\n",__func__);
	// AD7606_DEVICE *ad7606_dev = pfile->private_data;//获得设备结构体指针
	if (_IOC_TYPE(cmd) != AD7606_IOC_MAGIC)
		return -EINVAL;
	if (_IOC_NR(cmd) > AD7606_IOC_MAXNR) 
		return -EINVAL;
	switch(cmd){
		case AD7606_IOC_SET_ACQUSITION_PARA:
			copy_from_user(&ad7606_device.acqusition_para, (AD7606_ACQUSITION_PARA *) arg , sizeof(AD7606_ACQUSITION_PARA));
			PLOG("***get the ad7606_device.acqusition_para.freq is %d ***\n",ad7606_device.acqusition_para.freq);
			PLOG("***get the ad7606_device.acqusition_para.length is %d ***\n",ad7606_device.acqusition_para.length);
			channel_nums = ad7606_device.acqusition_para.valid_channel_nums;
			channel_list = ad7606_device.acqusition_para.valid_channel_list;
			PLOG("***get the valid_channel_nums is %d ***\n",channel_nums);
		
			break;
		default:
			return -EINVAL;
		break;
	}
	return 0;

}
//-----------------------------------------------------
static const struct file_operations ad7606_ops = {
		.owner 			= THIS_MODULE,
		.llseek			= ad7606_llseek,
		.open			= ad7606_open,
		.release		= ad7606_release,
		.unlocked_ioctl	= ad7606_ioctl,
		.read			= ad7606_read,
		.write			= ad7606_write
};
//----------------------------------------------------
static void ad7606_hrtimer_init(void)
{
	// pr_info("timer resolution: %lu\n", TICK_NSEC);
	// kt = ktime_set(0, 0); /* 1 sec, 10 nsec */
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = ad7606_hrtimer_handler;
	hrtimer_start(&timer, ktime_set(0, 0), HRTIMER_MODE_REL);
	// printk("\n-------- test start ---------\n");
}
//-----------------------------------------------------
static void ad7606_hrtimer_exit(void)
{
	hrtimer_cancel(&timer);
	// printk("-------- test over ----------\n");
}
//--------------------------------------------------------
static enum hrtimer_restart ad7606_hrtimer_handler(struct hrtimer *timer)
{
	short int s_adc_now[8];
	int i,j;
	char chl_nums,cur_chl;
	chl_nums = ad7606_device.acqusition_para.valid_channel_nums;
	if(cur_acqusition_length < ad7606_device.acqusition_para.length)
	{
		ad7606_Scan(s_adc_now);
		for (j = 0; j < chl_nums; ++j)
		{
			cur_chl = (*(ad7606_device.channel_info+j)).num;
			*((*(ad7606_device.channel_info+j)).data + cur_acqusition_length) = s_adc_now[cur_chl];
		}	

		hrtimer_forward(timer, timer->base->get_time(), kt);
		PLOG("***cur_acqusition_length: %d***\n",cur_acqusition_length);
		cur_acqusition_length++;
		return HRTIMER_RESTART;
	}
	else
	{
		finish_acquisition = 1;	
		cur_acqusition_length = 0;	
		wake_up_interruptible(&ad7606_waitq);  
		return HRTIMER_NORESTART;
	}
}
//----------------------------------------------------
static int __init ad7606_driver_init(void)
{
	int result;
	PLOG(KERN_INFO "****************ad7606 started*****************\n");

	ad7606_device.devno = MKDEV(AD7606_MAJOR,0);
	result = register_chrdev_region(ad7606_device.devno, 1, "ad7606");
	if (result < 0){
		PLOG("##register_chrdev_region:failed##\n");
		return -1;
	}
	//initialize and register device
	cdev_init(&(ad7606_device.s_cdev),&ad7606_ops);
	ad7606_device.s_cdev.owner = THIS_MODULE;
	result = cdev_add(&(ad7606_device.s_cdev), ad7606_device.devno, 1);
    if(result)
    {
        PLOG("##cdev_add:Error %d adding cdev\n##",result);
        return -1;
    }

	//create class
    ad7606_device.ad7606_dev_class = class_create(THIS_MODULE, AD7606_DEV_NAME);
	if (IS_ERR(ad7606_device.ad7606_dev_class)){
		PLOG("###class_create:failed to create class!###\n");
		return -1;
	}

	//create node
	device_create(ad7606_device.ad7606_dev_class, NULL,
			ad7606_device.devno, NULL, AD7606_DEV_NAME);

    PLOG("**ad7606 module initiation OK**\n");



	return 0;
}
module_init(ad7606_driver_init);
//----------------------------------------------------
static void __exit ad7606_driver_exit(void)
{
	
	device_destroy(ad7606_device.ad7606_dev_class, ad7606_device.devno);
	class_destroy(ad7606_device.ad7606_dev_class);
	cdev_del(&(ad7606_device.s_cdev));
	unregister_chrdev_region(MKDEV(AD7606_MAJOR,0),1);
	gpio_free_array(ad7606_gpios, ARRAY_SIZE(ad7606_gpios));
	
	PLOG(KERN_INFO "ad7606 closed\n ");

}
module_exit(ad7606_driver_exit);
//------------------------------------------------------
MODULE_AUTHOR("Blue <ge.blue@willtech-sh.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ad7606 driver");
MODULE_ALIAS("ad7606 driver");