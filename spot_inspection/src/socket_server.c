#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>


#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/tcp.h>


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
    int nbytes;
    static char readbuff[200];
    // PLOG("\n\n----------------------------new fork is starting------------------\n");
    PLOG("----------------------------here is socket server, waiting for connection...------------------\n");
    // PLOG("socket_server_interface.fifo_path is %s\n", socket_server_interface.fifo_path);
    socket_server_interface.port = port;
    socket_server_interface.socket_fd = socketServerAccept(socket_server_interface.server_fd);
    if (socket_server_interface.socket_fd < 0)
        return -1;
    socket_server_interface.is_connected = SocketConnected;
    PLOG("\n------------------------client is connected------------------------\n");

    connect_flag[SOCKET_SERVER_NUM]=1;
    system("/root/led.sh led_on tp-link:blue:system");	//light on the led
    // socket_server_interface.is_alive = 1;
    sendProductInfo(socket_server_interface.socket_fd);
    if (fork()==0)
    {
        //open fifo
        socket_server_interface.fifo_wr_fd = open(SOCKET_SERVER_FIFO_PATH,O_WRONLY | O_NONBLOCK);
        if (socket_server_interface.fifo_wr_fd == -1)
        {
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
		close(socket_server_interface.server_fd);
		close(socket_server_interface.fifo_wr_fd);
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


T_Client_Info t_client_info[MAX_CLIENT_NUM];
int serverForkInit(int port)
{
    if(fork()==0)
    {
        int connected_nums=0;
        int sockfd=-1;
        int reuse = 1;
        int sin_size;
        struct sockaddr_in server_addr;
        struct sockaddr_in client_addr;
        sin_size=sizeof(client_addr);


        if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:IPV4;SOCK_STREAM:TCP
        {
            fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
            exit(1);
        }
PLOG("after 000\n");

        bzero(&server_addr,sizeof(struct sockaddr_in)); //
        server_addr.sin_family=AF_INET;                 // Internet
        server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  //
        server_addr.sin_port=htons(port);


        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	PLOG("after 111\n");	
        if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
        {
            fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
            exit(1);
        }

		PLOG("after bind\n");
		
        if(listen(sockfd,MAX_CLIENT_NUM)==-1)
        {
            fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
            exit(1);
        }
		PLOG("after listen\n");
        while(1)
        {
            if((t_client_info[connected_nums].fd=accept(sockfd,(struct sockaddr *)&client_addr,(socklen_t *)&sin_size))==-1)
            {
                if(errno == EINTR)
                    continue;
                else
                {
                    fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
                    continue;
                }
            }
		 PLOG("after acceptr\n");
		memset(t_client_info[connected_nums].client_ip,'\0',16);
            strcpy(t_client_info[connected_nums].client_ip,inet_ntoa(client_addr.sin_addr));
            fprintf(stderr,"Server get connection from %s\n",t_client_info[connected_nums].client_ip); // 将网络地址转换成.字符串
		
    		sendProductInfo(t_client_info[connected_nums].fd);
		serverForkRun(t_client_info[connected_nums].fd);
		connected_nums++;
        }

    }
    else
        return 0;
}

void serverForkRun(int client_fd)
{
    if(fork()==0)
    {
        int nbytes=0;
        char readbuff[1024]= {0};
        int fifo_wr_fd;
        while(1)
        {
            PLOG("wait for data...\n");
            if((nbytes=read(client_fd,readbuff,1024))<=0)
            {
                fprintf(stderr,"Read Error:%s\n",strerror(errno));
                PLOG("%d, %s\n",nbytes,readbuff);
                close(client_fd);
                exit(1);
            }
            else
            {

                readbuff[nbytes]='\0';
                PLOG("nbytes is : %d, readbuff is :%s\n",nbytes,readbuff);
				
                fifo_wr_fd = open(DATA_FIFO_PATH,O_WRONLY | O_NONBLOCK);
                if (fifo_wr_fd == -1)
                {
                    PLOG("###Open %s ERROR!###",DATA_FIFO_PATH);
                    perror("###open fifo_wr_fd###");
                    exit(1);
                }


                if((nbytes = write(fifo_wr_fd, readbuff, nbytes)==-1))
                {
                    if(errno==EAGAIN)
                    {
                        PLOG("The SOCKET_SERVER_FIFO_PATH has not been read yet.Please try later\n");
                    }
                    else
                    {
                        PLOG("###write the fifo_wr_fd ERROR###\n");
                        perror("###fifo_wr_fd###");
                    }
                }
                else
                {
                    PLOG("write %s to the DATA_FIFO_PATH\n",readbuff);
                    close(fifo_wr_fd);
                }

            }

        }
    }		
     else
          close(client_fd);
}