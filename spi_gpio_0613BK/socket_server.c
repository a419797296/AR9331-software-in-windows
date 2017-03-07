#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include "socket_server.h"

//--------------------------------socket初始化-----------
int socketInit(int portnumber)
{
	int sockfd=-1; 
	
	struct sockaddr_in server_addr; 

	/* 服务器端开始建立sockfd描述符 */ 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:IPV4;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 

	/* 服务器端填充 sockaddr结构 */ 
	bzero(&server_addr,sizeof(struct sockaddr_in)); // 初始化,置0
	server_addr.sin_family=AF_INET;                 // Internet
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // (将本机器上的long数据转化为网络上的long数据)和任何主机通信  //INADDR_ANY 表示可以接收任意IP地址的数据，即绑定到所有的IP
	//server_addr.sin_addr.s_addr=inet_addr("192.168.1.1");  //用于绑定到一个固定IP,inet_addr用于把数字加格式的ip转化为整形ip
	server_addr.sin_port=htons(portnumber);         // (将本机器上的short数据转化为网络上的short数据)端口号
	
	/* 捆绑sockfd描述符到IP地址 */ 
	if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 

	/* 设置允许连接的最大客户端数 */ 
	if(listen(sockfd,5)==-1) 
	{ 
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno)); 
		exit(1); 
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
	sin_size=sizeof(struct sockaddr_in); 
	if((new_fd=accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size))==-1) 
	{ 
		fprintf(stderr,"Accept error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 
	fprintf(stderr,"Server get connection from %s\n",inet_ntoa(client_addr.sin_addr)); // 将网络地址转换成.字符串
	return new_fd;
}

//------------------------------连接socket服务器------------
int socketConnect(char * ip,int portnumber)
{
	/* 客户程序开始建立 sockfd描述符 */ 
	struct sockaddr_in server_addr; 
	struct hostent *host; 
	int sockfd; 
	if((host=gethostbyname(ip))==NULL) 
	{ 
		fprintf(stderr,"Gethostname error\n"); 
		exit(1); 
	} 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 

	/* 客户程序填充服务端的资料 */ 
	bzero(&server_addr,sizeof(server_addr)); // 初始化,置0
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber);  // (将本机器上的short数据转化为网络上的short数据)端口号
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); // IP地址
	
	/* 客户程序发起连接请求 */ 
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno)); 
		exit(1); 
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
		exit(1); 
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
		exit(1); 
	} 
	return nbytes;
}
//--------------------------------关闭socket链接-----------
void socketClose(int sockfd,int new_fd)
{
	close(new_fd); /* 结束通讯 */ 
	close(sockfd); 
}
