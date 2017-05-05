#ifndef SER2NET_H_
#define SER2NET_H_
#include "socket_driver.h"

#define SOCKET_SER2NET_IP			"127.0.0.1"
#define SOCKET_SER2NET_PORT			6666
#define SOCKET_SER2NET_PACK_NUMS			64   //if read in package model, this macro define the package nums


#define SOCKET_SER2NET_FIFO_PATH			"/tmp/socket_ser2net_fifo"
#define SOCKET_SER2NET_NUM			0


extern SOCKET_INTERFACE socket_ser2net_interface;

//-----------------------------------------------------------
#define TTY_DEV_PATH						"/dev/ttyATH0"	
#define TTY_FIFO_PATH						"/tmp/tty_fifo"
#define TTY_END_WITH_NULL					0
#define TTY_END_WITH_LENGTH				1
#define TTY_FIX_LENGTH						12
extern SOCKET_INTERFACE tty_interface;
int serToNetFork();
int ttyAth0Fork(int model) ;

#endif