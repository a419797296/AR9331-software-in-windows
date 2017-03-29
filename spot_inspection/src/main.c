/*
 * main.c
 *
 *  Created on: 2016-8-15
 *      Author: Blue <ge.blue@willtech-sh.com>
 * Description: This program is written for data requisition ,
 * 				 work with ad7606_driver
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
#include <sys/time.h>
#include <signal.h>
#include <time.h>

#include "main.h"
#include "ad7606_app.h"
#include "socket_driver.h"
#include "socket_client.h"
#include "socket_server.h"
#include "ser2net.h"
#include "socket_bussiness.h"

int shift_virb = 26;
int is_reset_para = 0;
int DEBUG = 0, MATLAB_TEST = 0;
int  length = 100, freq = 10, period = 1000;
int package_freq=20;
char enable_mode[3]="111";
char connect_flag[3]= {0, 0, 0};
char channel_nums = 8, channel_list[8] = {0, 1, 2, 3, 4, 5, 6, 7};
char cloud_ip[16] = "192.168.31.209";
int port = 3333;
char model[10] = "slave";
//For command options
static const char *optstring = "d:hl:f:c:vt:DS:m:Mp:e:";
static const char *usage = "\
Usage: gpio_ir_app [option] [option parameter]\n\
-h          display help information\n\
-d  <path>  device path\n\
-l  the length of the data\n\
-c  the ad channel_list\n\
-v  read the version of this software\n\
-t  sample interval\n\
-D  debug model, display the ad result\n\
-S  the server IP address\n\
-M  matlab test model\n\
-p  send package freq\n\
-e  enable the working model: enable the first bit means enable zigbee, second means server, third  means client; eg:111 means enable all\n\
";

static void systemInit(void);
static int parse_opt(const char opt, const char *optarg);
static int open_fifos(char * fifo_path, SOCKET_INTERFACE *socket_interface);
static int calc_shift_virb(void);
/* -------------------------------------------------------------------
 * Parse settings from app's command line.
 * ------------------------------------------------------------------- */
int parse_opt(const char opt, const char *optarg)
{
    int i;
    switch(opt)
    {
    case 'h':
        PLOG("%s\n",usage);
        exit(0);
        break;
    case 'd':
        PLOG("option:d\n");
        strcpy(ad7606_app.dev_path, optarg);
        break;
    case 'v':
        PLOG("Spot Inspection Version 1.0, 20160815\n");
        exit(0);
        break;
    case 'l':
        length = atoi(optarg);
        break;
    case 'f':
        freq = atoi(optarg);
        break;
    case 't':
        period = atoi(optarg);
        break;
    case 'D':
        DEBUG = 1;
        break;
    case 'm':
        strcpy(model, optarg);
        break;
    case 'M':
        MATLAB_TEST = 1;
        break;
    case 'S':
        strcpy(cloud_ip, optarg);
        break;
    case 'p':
        package_freq = atoi(optarg);
        break;
    case 'e':
        strcpy(enable_mode, optarg);
        break;
    case 'c':
        channel_nums = strlen(optarg); //减去结束标识
        PLOG("###channel_nums:%s, num is %d###\n",optarg, channel_nums);
        for (i = 0; i < channel_nums; ++i)
        {
            channel_list[i]= optarg[i] - '0';
        }
        break;
    default:
        PLOG("###Unknown option:%c###\n",opt);
        exit(0);
        break;
    }
    return 0;
}


//------------------------------------open the socket server, client, ser2net fork
int open_fifos(char * fifo_path, SOCKET_INTERFACE *socket_interface)
{
    //-----------------------------------------------------------------------check and open the socket_client fifo
    //check fifo file
    if (access(fifo_path,F_OK) == -1)
    {
        PLOG("**%s not exit,now create it**\n",fifo_path);
        if (mkfifo(fifo_path,0666) < 0)
        {
            PLOG("##Can't create fifo file!!##\n");
            return -1;
        }
    }
    /* 打开管道 */
    (*socket_interface).fifo_rd_fd=open(fifo_path,O_RDONLY|O_NONBLOCK);
    if((*socket_interface).fifo_rd_fd==-1)
    {
        perror("open");
        exit(1);
    }
    //else
    //PLOG("###Open %s successed!###\n",(*socket_interface).fifo_rd_fd);
    // PLOG("socket_interface.fifo_rd_fd is %d\n", (*socket_interface).fifo_rd_fd);
    strcpy((*socket_interface).fifo_path, fifo_path);
    return 0;
}

static void systemInit(void)
{
	if(system("pidof ser2net") !=0)
		{
		system("ser2net");
		PLOG("ser2net server has not started");
	}
		
	if(system("cat /proc/modules | grep ad7606")!=0)
		{
		system("insmod ad7606_driver");
		PLOG("the model of ad7606_driver has not be installed");
	}
	shift_virb = calc_shift_virb();
}
uint8 calc_system_soc(void)
{
    uint16 system_soc=0;
    set_acqusition_para(10, 1, 1, "2");
    channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, acqusition_para.length);		// free(channel_info);
    acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
    system_soc = *(channel_info->data);
    //PLOG("the system_soc is %d\n",system_soc);
    free(channel_info);
    if(system_soc > 4100)
        system_soc = 4100;
    if(system_soc < 3300)
        system_soc = 3300;
    system_soc = (system_soc - 3300) / 8;
    return (uint8)system_soc;
}

int calc_shift_virb(void)
{
    int virb_shitf=0;
	int Nums = 100;
	int i;
    set_acqusition_para(100, Nums, 1, "0");
    channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, acqusition_para.length);		// free(channel_info);
    acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
	for(i=0;i<Nums;i++)
		{
		virb_shitf+=*(channel_info->data+i);
	}
	virb_shitf /= Nums;
    return virb_shitf;
}

//----------------------------------------------main
int main(int argc,char *argv[])
{
    char opt;
    //int fifo_rd_fd;
    //char buf_r[200]= {0};
    //sleep(20);
    systemInit();
    //-----------------------------------------------------------------------
    ad7606_app.dev_fd 	= -1;
    strcpy(ad7606_app.dev_path, AD7606_DEVICE_PATH);

    //parse
    while((opt = getopt(argc, argv, optstring)) != -1)
    {

        parse_opt(opt, optarg);
    }


    /*if (access(DATA_FIFO_PATH,F_OK) == -1)
    {
        PLOG("**%s not exit,now create it**\n",DATA_FIFO_PATH);
        if (mkfifo(DATA_FIFO_PATH,0666) < 0)
        {
            PLOG("##Can't create fifo file!!##\n");
            exit(1);
        }
    }
	PLOG("1111111111\n");
    fifo_rd_fd=open(DATA_FIFO_PATH,O_RDONLY|O_NONBLOCK);
	PLOG("22222\n");
    if(fifo_rd_fd==-1)
    {
PLOG("333\n");
		perror("open");
        exit(1);
    }
	PLOG("444\n");*/

    //--------------------------------------------------------------------------open the socket fifo file
    open_fifos(SOCKET_SER2NET_FIFO_PATH, &socket_ser2net_interface);
    open_fifos(SOCKET_CLIENT_FIFO_PATH, &socket_client_interface);
    open_fifos(SOCKET_SERVER_FIFO_PATH, &socket_server_interface);
    //--------------------------------------------------------------------------open the ad7606 driver file

    ad7606_app.dev_fd = open(ad7606_app.dev_path, O_RDWR);
    if (ad7606_app.dev_fd == -1)
    {
        PLOG("###open %s ERROR###\n",ad7606_app.dev_path);
        perror("###open###");
        return -1;
    }
    //--------------------------------------------------------------------------init the server socket_fd
     socket_server_interface.server_fd = socketServerInitNoneBlock(port);
     if (socket_server_interface.server_fd < 0)
     {
         PLOG("soclet server init error\n");
         return -1;
     }


    //--------------------------------------------------------------------------open the forks of ser2net socket_client, socket_server
	if (enable_mode[0] == '1')
    {
        serToNetFork();
        //ttyAth0Fork(0);
        PLOG("tried to start the ser2net fork\n");
    }
    if (enable_mode[1] == '1')
    {
        socketServerFork(port);
         PLOG("tried to start the server fork\n");
        //serverForkInit(port);
       
    }
    if (enable_mode[2] == '1')
    {
        socketClientFork(cloud_ip, port);
        PLOG("tried to start the client fork\n");
    }
    socket_bussiness();
    return 0;
}
