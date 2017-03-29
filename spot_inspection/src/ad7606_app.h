/*
 * AD7606_app.h
 *
 *  Created on: 2015-5-10
 *      Author: robinson
 */

#ifndef AD7606_APP_H_
#define AD7606_APP_H_

#define AD7606_DEVICE_PATH		"/dev/inspection_ad7606"
// #define AD7606_FIFO_PATH			"/tmp/AD7606_fifo"
 #define AD7606DEV_SIZE			1024

#define MAX_RCV_BUF_SIZE			2000//Byte
#define MAX_BUF_SIZE				128//Byte

#define AD7606_IOC_MAGIC	'B'
#define AD7606_IOC_SET_ACQUSITION_PARA		_IOW(AD7606_IOC_MAGIC,0,AD7606_ACQUSITION_PARA)
#define AD7606_IOC_SET_FREQ		_IOW(AD7606_IOC_MAGIC,1,int)
#define AD7606_IOC_SET_LENGTH		_IOW(AD7606_IOC_MAGIC,2,int)
#define AD7606_IOC_SET_CHANEL		_IOW(AD7606_IOC_MAGIC,3,int)
#define AD7606_IOC_START	_IO(AD7606_IOC_MAGIC,4)

typedef struct{
	int 					num;
	char 					name[10];
	short int 				*data;
}AD7606_CHANNEL_INFO;

typedef struct{
	int 					freq;
	int 					length;	
	// int 					period;
	char 					valid_channel_nums;	
	char 					valid_channel_list[8];
}AD7606_ACQUSITION_PARA;

typedef struct{//AD7606_app class
	//public

	//private
	int		dev_fd;
	char	dev_path[256];
	// int 	fifo_fd;//file description of the fifo file
}AD7606_APP;

extern AD7606_APP 				ad7606_app;
extern AD7606_CHANNEL_INFO 	*channel_info;
extern AD7606_ACQUSITION_PARA 	acqusition_para;


AD7606_CHANNEL_INFO * malloc_result_buf(int channel_nums, int length);
int  set_acqusition_para(int freq, int length, int channel_nums, char *channel_list);
int acqusition_ad_data(int fd, AD7606_ACQUSITION_PARA acqusition_para, AD7606_CHANNEL_INFO *channel_info);

#endif /* AD7606_APP_H_ */

