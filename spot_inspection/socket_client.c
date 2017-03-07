#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>


#include "socket_driver.h"
#include "socket_client.h"
#include "com_tools.h"
#include "main.h"
// #include <signal.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>

SOCKET_INTERFACE socket_client_interface;
//----------------------------------客户端模式------
int socketClientFork(char *ip, int port)
{
	int nbytes,led_fp;
	static char readbuff[200];	

	strcpy(socket_client_interface.ip, ip);
	socket_client_interface.port = port; 

	PLOG("\n\n----------------------prepare connecting to the %s: %d---------------------\n",ip,port);
	socket_client_interface.socket_fd = socketConnect(ip,port); 
	if (socket_client_interface.socket_fd <0)
	{
		// PLOG("------------------connection failed---------------------\n");
		return -1;
	}
	//open fifo
	socket_client_interface.fifo_wr_fd = open(SOCKET_CLIENT_FIFO_PATH,O_WRONLY | O_NONBLOCK);

	if (socket_client_interface.fifo_wr_fd == -1){
		PLOG("###Open %s ERROR!###",SOCKET_CLIENT_FIFO_PATH);
		perror("###open fifo_wr_fd###");
		return -1;
	}
	// socket_client_interface.is_alive = 1; 
	PLOG("\n\n----------------------have connected to the %s: %d---------------------\n",ip,port);

	/*led_fp=open("/sys/class/leds/tp-link:blue:user/trigger",O_WRONLY);
	if (led_fp<0) return -1;
	write(led_fp,"switch0",7);
	close(led_fp);*/
	connect_flag[SOCKET_CLIENT_NUM]=1;
	system("/root/led.sh led_on tp-link:blue:system");	//light on the led

	sendProductInfo(socket_client_interface.socket_fd,"00:CA:01:0F:00:01");
	if (fork()==0)         
	{
		//socketWrite(cloud_socket_id,ad_result,2);
		PLOG("数据长度为%d\n",strlen(readbuff)); 
		while(1)
	    {
			if((nbytes=read(socket_client_interface.socket_fd,readbuff,200))<=0) 
			{ 
				fprintf(stderr,"Read Error:%s\n",strerror(errno)); 
				PLOG("%d, %s\n",nbytes,readbuff);
				PLOG("-------------------->与云服务器通讯异常！！！\n");

				close(socket_client_interface.socket_fd);
				exit(0);
			} 	
			else
			{
				PLOG("nbytes is : %d, readbuff is :%s\n",nbytes,readbuff);
				readbuff[nbytes]='\0';
				// PLOG("received data from cloud is: %s\n", readbuff);
					/* 向管道写入数据 */
				if((nbytes = write(socket_client_interface.fifo_wr_fd, readbuff, nbytes)==-1))
				{
					PLOG("###write the fifo_wr_fd ERROR###\n");
					perror("###fifo_wr_fd###");
					if(errno==EAGAIN)
						PLOG("The SOCKET_CLIENT_FIFO_PATH has not been read yet.Please try later\n");
				}
				// else 
				// 	PLOG("write %s to the SOCKET_CLIENT_FIFO_PATH\n",readbuff);
			}	

	    }
	}
	else
	{
		return 0;
	}
	
}
