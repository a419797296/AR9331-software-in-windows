/*
 * ad7606_app.c
 *
 *  Created on: 2015-5-10
 *      Author: robinson <wurobinson@joysince.com>
 * Description: This program is written for js9331 development board ,
 * 				 work with gpio_ir_driver
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/errno.h>

#include "ad7606_app.h"
#include "main.h"


AD7606_APP 				ad7606_app;
AD7606_CHANNEL_INFO 	*channel_info;
AD7606_ACQUSITION_PARA 	acqusition_para;

//-----------------------------------------------------------------------
int acqusition_ad_data(int fd, AD7606_ACQUSITION_PARA acqusition_para, AD7606_CHANNEL_INFO *channel_info)
{
	int i, j, length, read_nums;
	char channel_nums;
	int struct_length, data_length;

	channel_nums=acqusition_para.valid_channel_nums;
	length = acqusition_para.length;
	
	//------------------------------------------------------
	struct_length = channel_nums * (sizeof(AD7606_CHANNEL_INFO));
	data_length = sizeof(short int) * length;
	read_nums = struct_length + channel_nums * data_length;
	//-----------------------------------------------------------------start to get ad
	// gettimeofday(&tpstart,NULL);//开始时间

	if (read(ad7606_app.dev_fd, channel_info, read_nums) < 0)
	{
		PLOG("###read the kernal info ERROR###\n");
		perror("###read###");
		return -1;
	}

	if (DEBUG)
	{
		// PLOG("------------here is the channel information-----\n");
		// for (i = 0; i < (read_nums >> 1) ; ++i)
		// {
		// 	if (i % 10 == 0)
		// 	{
		// 		PLOG("\n");
		// 	}
		// 	PLOG("%6d",*((short int *)channel_info + i ));	
		// }
		// PLOG("\n-------------over-------\n");
		for (i = 0; i < channel_nums; ++i)
		{
			PLOG("------------here is the AD datas of channel %d -----\n", (*(channel_info+i)).num);
			for (j = 0; j < length; ++j)
			{
				PLOG("%6d",*((*(channel_info+i)).data+j));
				if ((j + 1) % 10 == 0)
					PLOG("\n");
			}
			PLOG("\n");
		}
	}
	return 0;
}

//--------------------------------------
int set_acqusition_para(int freq, int length, int channel_nums, char *channel_list)
{
	acqusition_para.freq = freq;
	acqusition_para.length = length;
	acqusition_para.valid_channel_nums = channel_nums;
	strcpy(acqusition_para.valid_channel_list , channel_list);

	//------------------------------------------------------------transmat the acqusition_para
	if (ioctl(ad7606_app.dev_fd, AD7606_IOC_SET_ACQUSITION_PARA, &acqusition_para) < 0)
	{
		PLOG("###ioctl AD7606_IOC_SET_FREQ ERROR###\n");
		perror("###ioctl###");
		return -1;
	}
	return 0;
}


//------------------------------------------
AD7606_CHANNEL_INFO * malloc_result_buf(int channel_nums, int length)
{
	int struct_length, data_length;
	int i,read_nums;


	struct_length = sizeof(AD7606_CHANNEL_INFO);
	data_length=sizeof(short int) * length;
	read_nums = channel_nums * (struct_length + data_length);
	channel_info = (AD7606_CHANNEL_INFO *)malloc(read_nums);//分配多通道数据结构
	PLOG("---the read_nums is %d ---\n", read_nums);
	for (i = 0; i < channel_nums; ++i)
	{
		(*(channel_info+i)).data=(short int *)(channel_info + channel_nums) +length * i;
		// PLOG("---(*(channel_info+%d)).data is %d ---\n",i, (char *)((*(channel_info+i)).data)-(char *)channel_info);
	}
	return channel_info;
}



