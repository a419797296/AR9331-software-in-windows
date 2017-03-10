#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <time.h>
#include <strings.h>
#include "socket_driver.h"


#define TIME_OUT_TIME 20 //connect超时时间20秒

//--------------------------------socket初始化-----------
int socketServerInitNoneBlock(int portnumber)
{
    int servfd;
    struct sockaddr_in servaddr;

    if((servfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        printf("Create socket error!\n");
        return -1;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(portnumber);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY); //inet_addr("192.168.0.1");

    int option=1;
    setsockopt(servfd,SOL_SOCKET,SO_REUSEADDR,(char *)&option,sizeof(option));
    struct linger li;
    li.l_onoff=1;
    li.l_linger=1;
    setsockopt(servfd,SOL_SOCKET,SO_LINGER,(char *)&li,sizeof(li));
    if(bind(servfd,(struct sockaddr * )&servaddr,sizeof(servaddr))<0)
    {
        perror("bind to port 5000 failure");
        return -1;
    }
    if(listen(servfd,1)<0)
    {
        perror("listen error");
        return -1;
    }

    int flags = fcntl(servfd, F_GETFL, 0);
    fcntl(servfd, F_SETFL, flags | O_NONBLOCK);  
    return servfd;
}
//--------------------------------socket初始化-----------
int socketServerAccept(int servfd)
 	// while(1)
{
    socklen_t len;
    int clifd;
    struct sockaddr_in cliaddr;
    len=sizeof(cliaddr);
    clifd=accept(servfd,(struct sockaddr *)&cliaddr,&len);
    if(clifd < 0)
    {
        if (errno==EAGAIN || errno == EWOULDBLOCK)
        {
            usleep(10000);
            // printf("no client is connect yet\n");
        }
        else
        {
            perror("call accept error"); 
        }
        return -1;
    }
    char szIp[17]; 
    bzero(szIp,17);
    inet_ntop(AF_INET,&cliaddr.sin_addr,szIp,16);
    printf("from client IP:%s,Port:%d\n",szIp,ntohs(cliaddr.sin_port));
    //char buf[256];
    //time_t t;
    //time(&t);
    //int datalen=sprintf(buf,"Server:%u\n",(unsigned int)t);
    //send(clifd,buf,datalen,0);

    // close(clifd);
    return clifd;
}    



//--------------------------------socket初始化-----------
int socketInit(int portnumber)
{
	int sockfd=-1; 
	int reuse = 1;
	struct sockaddr_in server_addr; 

	/* 服务器端开始建立sockfd描述符 */ 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:IPV4;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno)); 
		return -1;  
	} 

	/* 服务器端填充 sockaddr结构 */ 
	bzero(&server_addr,sizeof(struct sockaddr_in)); // 初始化,置0
	server_addr.sin_family=AF_INET;                 // Internet
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // (将本机器上的long数据转化为网络上的long数据)和任何主机通信  //INADDR_ANY 表示可以接收任意IP地址的数据，即绑定到所有的IP
	//server_addr.sin_addr.s_addr=inet_addr("192.168.1.1");  //用于绑定到一个固定IP,inet_addr用于把数字加格式的ip转化为整形ip
	server_addr.sin_port=htons(portnumber);         // (将本机器上的short数据转化为网络上的short数据)端口号
	

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	/* 捆绑sockfd描述符到IP地址 */ 
	if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno)); 
		return -1; 
	} 

	/* 设置允许连接的最大客户端数 */ 
	if(listen(sockfd,1)==-1) 
	{ 
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno)); 
		return -1;  
	} 
	return sockfd;


}
//--------------------------------socket建立连接-----------
int socketAccept(int sockfd)
{
	int new_fd=-1;  //默认错误
	int sin_size; 
	struct sockaddr_in client_addr; 
		/* 服务器阻塞,直到客户程序建立连接 */ 
	sin_size=sizeof(client_addr); 
	socketSetNonBlocking(sockfd);

	if((new_fd=accept(sockfd,(struct sockaddr *)&client_addr,(socklen_t *)&sin_size))==-1) 
	{ 
		fprintf(stderr,"Accept error:%s\n\a",strerror(errno)); 
		return -1; 
	} 
	fprintf(stderr,"Server get connection from %s\n",inet_ntoa(client_addr.sin_addr)); // 将网络地址转换成.字符串
	return new_fd;
}

//------------------------------连接socket服务器------------
int socketConnect(char * ip,int portnumber)
{
	// /* 客户程序开始建立 sockfd描述符 */ 
	// struct sockaddr_in server_addr; 
	// struct hostent *host; 
	// int sockfd; 
	// int error=-1, len,ret;
	// struct timeval tv; 
	// tv.tv_sec=10; 
	// fd_set rset,wset;
	// unsigned long ul = 1;
	

	// if((host=gethostbyname(ip))==NULL) 
	// { 
	// 	fprintf(stderr,"Gethostname error\n"); 
	// 	exit(1); 
	// } 
	// if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	// { 
	// 	fprintf(stderr,"Socket Error:%s\a\n",strerror(errno)); 
	// 	exit(1); 
	// } 
	// ioctl(sockfd, FIONBIO, &ul); //设置为非阻塞模式
	// /* 客户程序填充服务端的资料 */ 
	// bzero(&server_addr,sizeof(server_addr)); // 初始化,置0
	// server_addr.sin_family=AF_INET;          // IPV4
	// server_addr.sin_port=htons(portnumber);  // (将本机器上的short数据转化为网络上的short数据)端口号
	// server_addr.sin_addr=*((struct in_addr *)host->h_addr); // IP地址
	
	// /* 客户程序发起连接请求 */ 
	// if((ret=connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr)))==-1) 
	// { 
		
 //        if(EINPROGRESS!=errno)  
 //        {     
 //            /*if not fcntl->O_NDELAY, fall here*/  
 //            printf("connect() failed:%s/n",strerror(errno));   
 //            return 0;   
 //        } 
	// } 
 //    if(ret==0)  
 //    {   
 //        printf("connection ok at once/n");   
 //        close(sockfd);   
 //        return sockfd;   
 //    } 
 //    FD_ZERO(&rset);  
 //    FD_SET(sockfd,&rset); 

 //    if(-1==(ret=select(sockfd+1,&rset,&wset,NULL,&tv)))  
 //    {  
 //        close(sockfd);   
 //        printf("select error/n");   
 //        return 0;   
 //    }  
 //    if(0==ret)  
 //    {   
 //        close(sockfd);   
 //        printf("timeout/n");   
 //        return 0;   
 //    } 


 //    if(FD_ISSET(sockfd,&rset)||FD_ISSET(sockfd,&rset))  
 //    {  
 //        len=sizeof(error);  
 //        errno=0;  
 //        if(0>(ret=getsockopt(sockfd,SOL_SOCKET,SO_ERROR,(void*)&error,&len)))  
 //        {  
 //            printf("getsockopt error:%s/n",strerror(error));  
 //            close(sockfd);  
 //            return 0;  
 //        }  
 //        if(error!=0)  
 //        {   
 //            printf("getsockopt set error:%s/n",strerror(error));   
 //            close(sockfd);   
 //            return 0;   
 //        }  
 //        printf("getsockopt success/n");  
 //    }  
 //    else  
 //    {   
 //        close(sockfd);   
 //        printf("FD_ISSET error/n");   
 //        return 0;   
 //    }  
	// return sockfd;
		/* 客户程序开始建立 sockfd描述符 */ 
	struct sockaddr_in server_addr; 
	struct hostent *host; 
	int sockfd; 
	// unsigned long ul = 1;
	// while((host=gethostbyname(ip))==NULL&&h_errno==TRY_AGAIN)
	if((host=gethostbyname(ip))==NULL)
	{ 
		fprintf(stderr,"Gethostname error:%s, the ip addr is: %s\n",strerror(h_errno),ip); 
		return -1; 
	} 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno)); 
		return -1; 
	} 
	// socketSetNonBlocking(sockfd);
	/* 客户程序填充服务端的资料 */ 
	bzero(&server_addr,sizeof(server_addr)); // 初始化,置0
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber);  // (将本机器上的short数据转化为网络上的short数据)端口号
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); // IP地址

	
	/* 客户程序发起连接请求 */ 
	// if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	// ioctl(sockfd, FIONBIO, &ul); //设置为非阻塞模式
	/* 客户程序发起连接请求 */ 
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		if(EINPROGRESS!=errno)  
        {     
            /*if not fcntl->O_NDELAY, fall here*/  
            printf("connect() failed:%s\n",strerror(errno));   
            return -1;   
        } 
	} 

	return sockfd;

}
//--------------------------------socket接收-----------
int socketRead(int new_fd,char *readbuff,int lenth)
{
	int nbytes;
	if((nbytes=read(new_fd,readbuff,lenth))==-1) 
	{ 
		fprintf(stderr,"Read Error:%s\n",strerror(errno)); 
		close(new_fd);
		return -1; 
	} 		
	readbuff[nbytes]='\0';
	return nbytes;

}
//--------------------------------socket发送-----------
int socketWrite(int new_fd,unsigned char *writebuff,int lenth)
{
	int nbytes;
	writebuff[lenth]='\r';
	writebuff[lenth+1]='\n';
	if((nbytes=write(new_fd,writebuff,lenth+2))==-1) 
	{ 
		fprintf(stderr,"Read Error:%s\n",strerror(errno));
		close(new_fd); 
		return -1; 
	}
	return nbytes;
}

//--------------------------------socket发送-----------
int socketWriteNoEnd(int new_fd,char *writebuff,int lenth)
{
	int nbytes;
	if((nbytes=write(new_fd,writebuff,lenth))==-1) 
	{ 
		fprintf(stderr,"write Error:%s\n",strerror(errno));
		close(new_fd); 
		return -1; 
	} 
	return nbytes;
}

//--------------------------------socket发送-----------
int socketWriteByPackages(int new_fd,char *writebuff, int datalen, int package_num, int time_interval)
{
	int packages, residual;
	int i;
	packages = datalen / package_num;  //每包32个字节
	residual = datalen - packages * package_num;//+1 means sent '/0' together
	/*if (residual == 1)  //正好整包发完
	{
		packages --;
		residual = package_num + 1;
	}*/
	if (packages)  //the length is small then a package
	{
		for(i = 0; i < packages; i++)
		{
			if (socketWriteNoEnd(new_fd, writebuff + i * package_num, package_num) < 0)
				return -1;
			usleep(time_interval);
		}
	}

	if (socketWriteNoEnd(new_fd, writebuff + packages * package_num, residual) < 0)
		return -1;
	//printf("packages = %d, residual = %d\n", packages, residual);


	return 0;
}

//--------------------------------socket发送-----------
int socketWriteWithEnd(int new_fd,char *writebuff,int lenth,char *end, int end_length)
{
	int nbytes, i;
	for (i = 0; i < end_length; ++i)
	{
		*(writebuff+lenth + i) = *(end + i); 
	}
	if((nbytes = write(new_fd,writebuff,lenth+end_length))==-1) 
	{ 
		fprintf(stderr,"Read Error:%s\n",strerror(errno));
		close(new_fd); 
		return -1; 
	} 
	return nbytes;
}
//--------------------------------关闭socket链接-----------
void socketClose(int sockfd,int new_fd)
{
	close(new_fd); /* 结束通讯 */ 
	close(sockfd); 
}
//-----------------------------设置超时时间
int set_sock_time(int fd, int read_sec, int write_sec)  
{  
    struct timeval send_timeval;  
    struct timeval recv_timeval;  
    if(fd <= 0) return -1;  
    send_timeval.tv_sec = write_sec<0?0:write_sec;  
    send_timeval.tv_usec = 0;  
    recv_timeval.tv_sec = read_sec<0?0:read_sec;;  
    recv_timeval.tv_usec = 0;  
    if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &send_timeval, sizeof(send_timeval)) == -1)  
    {  
        return -1;  
    }  
    if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeval, sizeof(recv_timeval)) == -1)  
    {  
        return -1;  
    }  
    return 0;  
}  

int SocketWait(int fd,int rd,int wr,int timems)
{
     fd_set rfds,wfds;
     struct timeval tv;
     FD_ZERO(&rfds);
     FD_ZERO(&wfds);
     if (rd)     //TRUE
          FD_SET(fd,&rfds);   //添加要测试的描述字
     if (wr)     //FALSE
          FD_SET(fd,&wfds);
     tv.tv_sec=timems/1000;     //second
     tv.tv_usec=timems%1000;     //ms
     for (;;) //如果errno==EINTR，反复测试缓冲区的可读性
         switch(select(fd+1,&rfds,&wfds,NULL,&tv))//测试在规定的时间内套接口接收缓冲区中是否有数据可读
         {                                              //0－－超时，-1－－出错
         case 0:     /* time out */
              return 0;
         case (-1):    /* socket error */
              if (errno==EINTR)
                   break;
              return 0; //有错但不是EINTR
          default:
              if (FD_ISSET(fd,&rfds)) //如果s是fds中的一员返回非0，否则返回0
                   return 1;
              if (FD_ISSET(fd,&wfds))
                   return 2;
              return 0;
         }
}


//-----------------------------------检测是否断开
int SocketConnected(int sock)
{
    if (sock <= 0)
        return 0;
    struct tcp_info info;
    int len = sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) & len);
    if ((info.tcpi_state == TCP_ESTABLISHED)) {
        //myprintf("socket connected\n");
        return 1;
    } else {
        //myprintf("socket disconnected\n");
        return 0;
    }
}


//-----------------------------------设置socket非阻塞  
void socketSetNonBlocking(int sock)  
{  
    int opts;  
    opts = fcntl(sock, F_GETFL);  
    if (opts < 0)  
    {  
        perror("fcntl(sock, GETFL)");  
        exit(1);  
    }  
    opts = opts|O_NONBLOCK;  
    if (fcntl(sock, F_SETFL, opts) < 0)  
    {  
        perror("fcntl(sock, SETFL, opts)");  
        exit(1);  
    }  
}

//-----------------------------------

// int connect_nonb(int sockfd, struct sockaddr_in *saptr, socklen_t salen, int nsec)
// {
//     int     flags, n, error;
//     socklen_t len;
//     fd_set rset, wset;
//     struct timeval tval;
//  
//     flags = Fcntl(sockfd, F_GETFL, 0);
//     fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
//  
//     error = 0;
//     if ( (n = connect(sockfd, saptr, salen)) < 0)
//         if (errno != EINPROGRESS)
//             return (-1);
//  
//     /* Do whatever we want while the connect is taking place. */
//  
//     if (n == 0)
//         goto done;               /* connect completed immediately */
//  
//     FD_ZERO(&rset);
//     FD_SET(sockfd, &rset);
//     wset = rset;
//     tval.tv_sec = nsec;
//     tval.tv_usec = 0;
//  
//     if ( (n = Select(sockfd + 1, &rset, &wset, NULL,
//                     nsec ? &tval : NULL)) == 0) {
//         close(sockfd);          /* timeout */
//         errno = ETIMEDOUT;
//         return (-1);
//     }
//  
//     if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
//         len = sizeof(error);
//         if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
//             return (-1);     /* Solaris pending error */
//     } else
//         err_quit("select error: sockfd not set");
//  
//   done:
//     fcntl(sockfd, F_SETFL, flags);  /* restore file status flags */
//  
//     if (error) {
//         close(sockfd);           /* just in case */
//         errno = error;
//         return (-1);
//     }
//     return (0);
// }