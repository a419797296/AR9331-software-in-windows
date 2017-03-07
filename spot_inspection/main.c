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
        printf("**%s not exit,now create it**\n",fifo_path);
        if (mkfifo(fifo_path,0666) < 0)
        {
            printf("##Can't create fifo file!!##\n");
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
    // printf("socket_interface.fifo_rd_fd is %d\n", (*socket_interface).fifo_rd_fd);
    strcpy((*socket_interface).fifo_path, fifo_path);
    return 0;
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
	if(system_soc > 4200)
		system_soc = 4200;
	if(system_soc < 3200)
		system_soc = 3200;
	system_soc = (system_soc - 3200) / 10;
	return (uint8)system_soc;
}

//----------------------------------------------main
int main(int argc,char *argv[])
{
    char opt;
    char buf_r[200]= {0};
    //sleep(20);
    //-----------------------------------------------------------------------
    ad7606_app.dev_fd 	= -1;
    strcpy(ad7606_app.dev_path, AD7606_DEVICE_PATH);

    //parse
    while((opt = getopt(argc, argv, optstring)) != -1)
    {

        parse_opt(opt, optarg);
    }

    //--------------------------------------------------------------------------open the socket fifo file
    open_fifos(SOCKET_SER2NET_FIFO_PATH, &socket_ser2net_interface);
    //open_fifos(TTY_FIFO_PATH, &tty_interface);
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
        printf("soclet server init error\n");
        return -1;
    }

    /*tty_interface.fifo_rd_fd = open(TTY_DEV_PATH,O_RDWR);
    	if (tty_interface.fifo_rd_fd == -1){
    	PLOG("###Open %s ERROR!###\n",TTY_DEV_PATH);
    	perror("###open fifo_wr_fd###\n");
    	return -1;
    }
    	else
    		{
    		PLOG("###Open %s successed!###\n",TTY_DEV_PATH);
    		while(1)
    			{
    			nread=read((tty_interface).fifo_rd_fd,buf_r,200);
    			buf_r[nread] = 0;
    			PLOG("###read  %d bytes!###, :%s\n",nread,buf_r);
    		}

    	}*/
    	


    //--------------------------------------------------------------------------open the forks of ser2net socket_client, socket_server
    if (enable_mode[0] == '1')
    {
        serToNetFork();
        //ttyAth0Fork(0);
        printf("tried to start the ser2net fork\n");
    }
    if (enable_mode[1] == '1')
    {
        socketServerFork(port);
        printf("tried to start the server fork\n");
    }
    if (enable_mode[2] == '1')
    {
        socketClientFork(cloud_ip, port);
        printf("tried to start the client fork\n");
    }
    socket_bussiness();
    return 0;
}
