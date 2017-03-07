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
#include "socket_server.h"
#include "com_tools.h"
#include "main.h"
// #include <signal.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>

// SOCKET_SERVER_INTERFACE socket_server_interface;
SOCKET_INTERFACE socket_server_interface;
//----------------------------------服务器模式------
int socketServerFork(int port)
{
	int nbytes,led_fp;
	static char readbuff[200];	
	char macAddrBuff[18]={0};
	// PLOG("\n\n----------------------------new fork is starting------------------\n");
	PLOG("----------------------------here is socket server, waiting for connection...------------------\n");
	// PLOG("socket_server_interface.fifo_path is %s\n", socket_server_interface.fifo_path);
	socket_server_interface.port = port; 
	socket_server_interface.socket_fd = socketServerAccept(socket_server_interface.server_fd);
	if (socket_server_interface.socket_fd < 0)
		return -1;
	socket_server_interface.is_connected = SocketConnected;
	PLOG("\n------------------------client is connected------------------------\n");

	/*led_fp=open("/sys/class/leds/tp-link:blue:user/trigger",O_WRONLY);
	if (led_fp<0) return -1;
	write(led_fp,"switch0",7);
	close(led_fp);*/
	connect_flag[SOCKET_SERVER_NUM]=1;
	system("/root/led.sh led_on tp-link:blue:system");	//light on the led
	// socket_server_interface.is_alive = 1;
	getMacAddr("eth1",macAddrBuff);
	sendProductInfo(socket_server_interface.socket_fd,macAddrBuff);
	if (fork()==0)         
	{
		//open fifo
		socket_server_interface.fifo_wr_fd = open(SOCKET_SERVER_FIFO_PATH,O_WRONLY | O_NONBLOCK);
		if (socket_server_interface.fifo_wr_fd == -1){
			PLOG("###Open %s ERROR!###",SOCKET_SERVER_FIFO_PATH);
			perror("###open fifo_wr_fd###");
			return -1;
		}

		while(1)
	    {
			PLOG("wait for data...\n");
			if((nbytes=read(socket_server_interface.socket_fd,readbuff,200))<=0) 
			{ 
				fprintf(stderr,"Read Error:%s\n",strerror(errno)); 
				PLOG("%d, %s\n",nbytes,readbuff);
				PLOG("-------------------->与云服务器通讯异常！！！\n");
				close(socket_server_interface.socket_fd);
				exit(0);
			} 	
			else
			{
				PLOG("nbytes is : %d, readbuff is :%s\n",nbytes,readbuff);
				readbuff[nbytes]='\0';
				if((nbytes = write(socket_server_interface.fifo_wr_fd, readbuff, nbytes)==-1))
				{
					PLOG("###write the fifo_wr_fd ERROR###\n");
					perror("###fifo_wr_fd###");
					if(errno==EAGAIN)
						PLOG("The SOCKET_SERVER_FIFO_PATH has not been read yet.Please try later\n");
				}
				// else 
				// 	PLOG("write %s to the SOCKET_SERVER_FIFO_PATH\n",readbuff);
			}	
	    }
	}
	else
	{
		return 0;
	}
}