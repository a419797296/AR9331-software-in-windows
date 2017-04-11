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
#include "com_tools.h"

int shift_virb = 27;
//int is_reset_para = 0;
int DEBUG = 0;
int package_freq=20;
char enable_mode[3]="000";
char connect_flag[3]= {0, 0, 0};
char channel_nums = 8, channel_list[8] = {0, 1, 2, 3, 4, 5, 6, 7};



PRODUCT_INFO produc_info;
//For command options
static const char *optstring = "hvf:e:DS:P:C";
static const char *usage = "\
Usage: gpio_ir_app [option] [option parameter]\n\
-h          display help information\n\
-v  read the version of this software\n\
-f  freq of sending package \n\
-e  enable the working model: enable the first bit means enable zigbee, second means server, third  means client; eg:111 means enable all\n\
-D  debug model, display the ad result\n\
-S  the server IP address\n\
-P  the server PORT\n\
-C  calibrate the virb shift\n\
";
static void DevAndFifoFileInit(void);
static void ProductInfoInit(void);
static void EnvironmentInit(void);
static void paraInit(void);
static void systemInit(void);
static int parse_opt(const char opt, const char *optarg);
static int open_fifos(char * fifo_path, SOCKET_INTERFACE *socket_interface);

/* -------------------------------------------------------------------
 * Parse settings from app's command line.
 * ------------------------------------------------------------------- */
int parse_opt(const char opt, const char *optarg)
{
	char cloud_ip[16] = {0};
	switch(opt)
    {
    case 'h':
        PLOG("%s\n",usage);
        exit(0);
        break;
    case 'v':
        PLOG("produc_info.mac:%s,produc_info.hw_vers:%s,produc_info.sw_vers :%s\n",produc_info.mac,produc_info.hw_vers,produc_info.sw_vers );
        exit(0);
        break;
    case 'D':
        DEBUG = 1;
        break;
    case 'S':
	
        strcpy(cloud_ip, optarg);
	setSysUciCfgStr("spot_inspection","cloud","ip",cloud_ip);
        break;
    case 'P':
	setSysUciCfgNum("spot_inspection","cloud","ip",atoi(optarg));
        break;
    case 'f':
	setSysUciCfgNum("spot_inspection","para","package_freq",atoi(optarg));
        break;
    case 'e':
        strcpy(enable_mode, optarg);
	setSysUciCfgStr("spot_inspection","para","enable_mode",enable_mode);
        break;
    case 'C':
	setSysUciCfgNum("spot_inspection","para","virb_shift",calc_shift_virb());
	exit(0);
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
//-----------------------------------------------

static void EnvironmentInit(void)
{
	if(system("pidof ser2net") !=0)	//check the ser2net is up
		{
		system("ser2net");
		PLOG("ser2net server has not started");
	}
		
	if(system("cat /proc/modules | grep ad7606")!=0)  //check the ad7606_driver is installed
		{
		system("insmod ad7606_driver");
		PLOG("the model of ad7606_driver has not be installed");
	}
	system("/root/led.sh blink_slow tp-link:blue:system");   //set the led flash slow, means the software is ready
}
//-----------------------------------------------

static void ProductInfoInit(void)
{
	  memset(produc_info.hw_vers, 0, sizeof(produc_info.hw_vers));
	  memset(produc_info.sw_vers, 0, sizeof(produc_info.sw_vers));
	  memset(produc_info.mac, 0, sizeof(produc_info.mac));
	  if(getMacAddr("eth0",produc_info.mac) == -1)
	  {
	      printf("read the mac error: readed data is %s\n", produc_info.mac);
	  }
	  getSysUciCfgStr("spot_inspection","product_info","hw_vers",produc_info.hw_vers);
	  getSysUciCfgStr("spot_inspection","product_info","sw_vers",produc_info.sw_vers);

	  PLOG("produc_info.mac:%s,produc_info.hw_vers:%s,produc_info.sw_vers :%s\n",produc_info.mac,produc_info.hw_vers,produc_info.sw_vers );
}
//-----------------------------------------------
static void DevAndFifoFileInit(void)
{
//-----------------------------------------------------------------------
    ad7606_app.dev_fd 	= -1;
    strcpy(ad7606_app.dev_path, AD7606_DEVICE_PATH);
	//--------------------------------------------------------------------------open the ad7606 driver file

    ad7606_app.dev_fd = open(ad7606_app.dev_path, O_RDWR);
    if (ad7606_app.dev_fd == -1)
    {
        PLOG("###open %s ERROR###\n",ad7606_app.dev_path);
        perror("###open###");
        exit(1);
    }
	
    //--------------------------------------------------------------------------open the socket fifo file
    open_fifos(SOCKET_SER2NET_FIFO_PATH, &socket_ser2net_interface);
    open_fifos(SOCKET_CLIENT_FIFO_PATH, &socket_client_interface);
    open_fifos(SOCKET_SERVER_FIFO_PATH, &socket_server_interface);
}

//-----------------------------------------------
static void paraInit(void)
{
	getSysUciCfgStr("spot_inspection","para","enable_mode",enable_mode);
	getSysUciCfgNum("spot_inspection","para","package_freq");
	getSysUciCfgNum("spot_inspection","para","virb_shift");
}

//-----------------------------------------------
static void systemInit(void)
{
	paraInit();
	DevAndFifoFileInit();
	ProductInfoInit();
	EnvironmentInit();
	shift_virb = getSysUciCfgNum("spot_inspection","para","virb_shift"); 
	PLOG("the shift_virb is %d",shift_virb);

	//--------------------------------------------------------------------------init the server socket_fd
     socket_server_interface.server_fd = socketServerInitNoneBlock(3333);
     if (socket_server_interface.server_fd < 0)
     {
         PLOG("soclet server init error\n");
         exit(1);
     }
}


//----------------------------------------------main
int main(int argc,char *argv[])
{
    char opt;
	char  cloud_ip[16] = {0};
	int cloud_port;
	systemInit();
	
	while((opt = getopt(argc, argv, optstring)) != -1)
    {

        parse_opt(opt, optarg);
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
        socketServerFork(3333);
         PLOG("tried to start the server fork\n");
        //serverForkInit(port);
       
    }
    if (enable_mode[2] == '1')
    {
	cloud_port = getSysUciCfgNum("spot_inspection","cloud","cloud_port");
	getSysUciCfgStr("spot_inspection","cloud","ip",cloud_ip);
	socketClientFork(cloud_ip, cloud_port);
        PLOG("tried to start the client fork\n");
    }
    socket_bussiness();
    return 0;
}
