#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ser2net.h"
#include "socket_driver.h"
#include "main.h"
SOCKET_INTERFACE socket_ser2net_interface;
SOCKET_INTERFACE tty_interface;
//----------------------------------serToNet服务进程------
int serToNetFork()
{
    int nbytes;
    int allNum=0;
    char readbuff[256];
    char package[SOCKET_SER2NET_PACK_NUMS];
    strcpy(socket_ser2net_interface.ip, SOCKET_SER2NET_IP);
    socket_ser2net_interface.port = SOCKET_SER2NET_PORT;
    bool fullPackaged = 0;


    /* 使用hostname查询host 名字 */
    socket_ser2net_interface.socket_fd=socketConnect(SOCKET_SER2NET_IP,SOCKET_SER2NET_PORT);
    if (socket_ser2net_interface.socket_fd <0)
    {
        PLOG("------------------connection failed---------------------\n");
        return -1;
    }

    PLOG("\n\n----------------------ser2net have connected to the %s: %d---------------------\n",SOCKET_SER2NET_IP,SOCKET_SER2NET_PORT);

    connect_flag[SOCKET_SER2NET_NUM]=1;

    socket_ser2net_interface.fifo_wr_fd = open(SOCKET_SER2NET_FIFO_PATH,O_WRONLY | O_NONBLOCK);

    if (socket_ser2net_interface.fifo_wr_fd == -1)
    {
        PLOG("###Open %s ERROR!###",SOCKET_SER2NET_FIFO_PATH);
        perror("###open fifo_wr_fd###");
        return -1;
    }

    if (fork()==0)
    {
        while(1)
        {
            memset(package,'\0',SOCKET_SER2NET_PACK_NUMS);
            if((nbytes=read(socket_ser2net_interface.socket_fd, package, SOCKET_SER2NET_PACK_NUMS)) <= 0)
            {
                fprintf(stderr,"Read Error:%s\n",strerror(errno));
                PLOG("%d, %s\n",nbytes,readbuff);
                PLOG("-------------------->与ser2net服务器通讯异常！！！\n");
                close(socket_ser2net_interface.socket_fd);
                exit(0);
            }
            else
            {
#ifdef SOCKET_SER2NET_END_WITH_NULL
                strncpy(readbuff + allNum, package, nbytes);
                // PLOG("the current package is %s\n", package);
                PLOG("the current readbuff is %s\n", readbuff);
                allNum += nbytes;

                if (package[nbytes - 1] == '\0')   //读到的最后一位不为零, 读取完毕
                {
                    fullPackaged = 1;
                }
#endif

#ifdef SOCKET_SER2NET_END_WITH_LENGTH

		
		  memcpy(readbuff + allNum, package, nbytes);
                // PLOG("the current package is %s\n", package);
                PLOG("the current readbuff is %s\n", readbuff);
                allNum += nbytes;
				
                if(readbuff[0]!=0x16)
                {
			PLOG("first byte error :%x\n",readbuff[0]);
			allNum = 0;
                    memset(readbuff, '\0', sizeof(readbuff));
			continue;
                }				
				
                if(allNum >= SOCKET_SER2NET_FIX_LENGTH)
                {
                    PLOG("nbytes is : %d, readbuff is :%s\n",allNum,readbuff);
                    int i=0;
                    unsigned char xor=0;
                    for(i=0; i<allNum; i++)
                    {
                        PLOG("%02x ",(unsigned char)readbuff[i]);
                        if(i<11)
                            xor-=readbuff[i];

                    }
                    PLOG("\nthe xor is %02x \n",xor);
                    /*PLOG("\nthe soc is %02x \n",soc);*/
                    if(xor==(unsigned char)readbuff[11])
                    {
                        fullPackaged =1;
                    }
                    else
                    {
                        allNum = 0;
                        memset(readbuff, '\0', sizeof(readbuff));
                    }
                }


#endif
                if(fullPackaged)
                {
                    fullPackaged = 0;
                    PLOG("nbytes is : %d, readbuff is :%s\n",allNum,readbuff);										/* 向管道写入数据 */
                    if((nbytes = write(socket_ser2net_interface.fifo_wr_fd, readbuff, allNum + 1)==-1))    //+1表示发送结束标识符
                    {
                        PLOG("###write the fifo_wr_fd ERROR###\n");
                        perror("###fifo_wr_fd###");
                        if(errno==EAGAIN)
                            PLOG("The SOCKET_CLIENT_FIFO_PATH has not been read yet.Please try later\n");
                    }
                    allNum = 0;
                    memset(readbuff, '\0', sizeof(readbuff));
                }

            }

        }
    }
    else
    {
        return 0;
    }

}

int ttyAth0Fork(int model)
{
    int nbytes;
    int allNum=0;
    char readbuff[1024];
    char package[SOCKET_SER2NET_PACK_NUMS];
    bool fullPackaged = 0;

    tty_interface.socket_fd = open(TTY_DEV_PATH,O_RDWR);
    if (tty_interface.socket_fd == -1)
    {
        PLOG("###Open %s ERROR!###\n",TTY_DEV_PATH);
        perror("###open fifo_wr_fd###\n");
        return -1;
    }
    else
    {
        PLOG("###Open %s successed!###\n",TTY_DEV_PATH);

    }

    tty_interface.fifo_wr_fd = open(TTY_FIFO_PATH,O_WRONLY | O_NONBLOCK);

    if (tty_interface.fifo_wr_fd == -1)
    {
        PLOG("###Open %s ERROR!###",TTY_FIFO_PATH);
        perror("###open fifo_wr_fd###");
        return -1;
    }

    if (fork()==0)
    {
        while(1)
        {
            memset(package,'\0',SOCKET_SER2NET_PACK_NUMS);
            if((nbytes=read(tty_interface.socket_fd, package, SOCKET_SER2NET_PACK_NUMS)) <= 0)
            {
                fprintf(stderr,"Read Error:%s\n",strerror(errno));
                PLOG("%d, %s\n",nbytes,readbuff);
                PLOG("-------------------->与ser2net服务器通讯异常！！！\n");
                close(tty_interface.socket_fd);
                exit(0);
            }
            else
            {

                strncpy(readbuff + allNum, package, nbytes);
                // PLOG("the current package is %s\n", package);
                PLOG("the current readbuff is %s\n", readbuff);
                allNum += nbytes;
                if(model == TTY_END_WITH_NULL)
                {
                    if (package[nbytes - 1] == '\0')
                    {
                        fullPackaged =1;
                        PLOG("nbytes is : %d, readbuff is :%s\n",allNum,readbuff);

                    }

                }

                if(model == TTY_END_WITH_LENGTH)
                {
                    if(allNum >= TTY_FIX_LENGTH)
                    {
                        PLOG("nbytes is : %d, readbuff is :%s\n",allNum,readbuff);
                        int i=0;
                        unsigned char soc=0;
                        for(i=0; i<allNum; i++)
                        {
                            PLOG("%02x ",(unsigned char)readbuff[i]);
                            if(i<11)
                                soc-=readbuff[i];

                        }
                        PLOG("\nthe soc is %02x \n",(unsigned char)soc);
                        PLOG("\nthe soc is %02x \n",soc);
                        if(readbuff[0]==0x16&&soc==(unsigned char)readbuff[11])
                        {
                            fullPackaged =1;
                        }
                    }
                    else       //if the first data is not 0x16,then clear all the data
                    {
                        if(readbuff[0]!=0x16)
                        {
                            allNum = 0;
                            memset(readbuff, '\0', sizeof(readbuff));
                        }
                    }

                }

                if(fullPackaged)  //if one package data is ready, then send to the fifo
                {
                    fullPackaged=0;   //clear the flag
                    if((nbytes = write(tty_interface.fifo_wr_fd, readbuff, allNum + 1)==-1))    //+1表示发送结束标识符
                    {
                        PLOG("###write the fifo_wr_fd ERROR###\n");
                        perror("###fifo_wr_fd###");
                        if(errno==EAGAIN)
                            PLOG("The SOCKET_CLIENT_FIFO_PATH has not been read yet.Please try later\n");
                    }
                    allNum = 0;
                    memset(readbuff, '\0', sizeof(readbuff));
                }
            }
        }

    }
    else
    {
        return 0;
    }

}








