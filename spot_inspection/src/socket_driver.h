#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_
// #include <sys/socket.h> 

typedef struct{
	int		server_fd;
	int		socket_fd;
	char 	ip[16];
	int 	port;
	int 	fifo_wr_fd;//file description of the fifo file
	int 	fifo_rd_fd;//file description of the fifo file
	char	fifo_path[50];
	int 	pipe_fd[2];//用于父子进程间通讯，主要传递socket文件描述符
	int 	(*is_connected)(int);
}SOCKET_INTERFACE;

int socketInit(int portnumber);
int socketAccept(int sockfd);
int socketRead(int new_fd,char *readbuff,int lenth);
int socketWrite(int new_fd,unsigned char *writebuff,int lenth);
void socketClose(int sockfd,int new_fd);
int socketConnect(char * ip,int portnumber);
int socketWriteNoEnd(int new_fd,char *writebuff,int lenth);
int set_sock_time(int fd, int read_sec, int write_sec);
int SocketWait(int fd,int rd,int wr,int timems);
int socketWriteWithEnd(int new_fd,char *writebuff,int lenth,char *end, int end_length);
int SocketConnected(int sock);
void socketSetNonBlocking(int sock);  
int socketWriteByPackages(int new_fd,char *writebuff, int datalen, int package_num, int time_interval);
int socketServerInitNoneBlock(int portnumber);
int socketServerAccept(int servfd);
// int connect_nonb(int sockfd, struct sockaddr_in *saptr, socklen_t salen, int nsec);
#endif

