#ifndef SOCKET_SERVER_FORK_H_
#define SOCKET_SERVER_FORK_H_

#define SOCKET_SERVER_FIFO_PATH			"/tmp/socket_server_fifo"
#define SOCKET_SERVER_NUM			1
// #include "socket_driver.h"

extern SOCKET_INTERFACE socket_server_interface;
int socketServerFork(int port);

#endif