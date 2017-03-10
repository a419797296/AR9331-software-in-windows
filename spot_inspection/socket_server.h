#ifndef SOCKET_SERVER_FORK_H_
#define SOCKET_SERVER_FORK_H_

#include "main.h"

#define SOCKET_SERVER_FIFO_PATH			"/tmp/socket_server_fifo"
#define SOCKET_SERVER_NUM			1
// #include "socket_driver.h"

extern SOCKET_INTERFACE socket_server_interface;
int socketServerFork(int port);
int serverForkInit(int port);
void serverForkRun(int client_fd);


typedef struct{
	int		fork_id;
	char		client_name[20];
	char 	client_ip[16];
	int 		fd;
	int 		stat;//file description of the fifo file
}T_Client_Info, *PT_Client_Info;


#endif