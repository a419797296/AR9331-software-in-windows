#ifndef SOCKET_CLIENT_FORK_H_
#define SOCKET_CLIENT_FORK_H_

#define SOCKET_CLIENT_FIFO_PATH			"/tmp/socket_client_fifo"
#define SOCKET_CLIENT_NUM			2

typedef struct{
	int		socket_fd;
	char 	ip[16];
	int 	port;
	int 	fifo_wr_fd;//file description of the fifo file
	int 	fifo_rd_fd;//file description of the fifo file
	int 	pipe_fd[2];//用于父子进程间通讯，主要传递socket文件描述符
	int 	is_alive;
}SOCKET_CLIENT_INTERFACE;

extern SOCKET_INTERFACE socket_client_interface;
int socketClientFork(char *ip, int port);
// enum jsonType{chatInfo,wifiInfo, controlInfo,dataReport};
// void doit(char *receivedData,int sockfd);
// int doControlInfo(char *receivedData,unsigned char *ad_result);
// void scoketClientFork();
// void zigbeeConfig(char *receivedData,unsigned char *ad_result);
// enum jsonType judgeJsonType(char * receivedData);
// void signal_func(int sign_no);
#endif