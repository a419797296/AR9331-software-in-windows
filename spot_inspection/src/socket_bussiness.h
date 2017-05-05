#ifndef SOCKET_CLIENT_BUSSINESS
#define SOCKET_CLIENT_BUSSINESS
#include "com_tools.h"   //pow_of_two(int num)
#include "main.h" 
#define MAX_LENGTH_BY_ONCE						1000
// #define SHIFT_BIT_TO_PACKAGE_LENGTH				pow_of_two(package_freq)
// #define PACKAGE_LENGTH							(1 << SHIFT_BIT_TO_PACKAGE_LENGTH)


/*#define GETWAY_TO_ZIGBEE	1
#define ZIGBEE_TO_GETWAY	2
#define CONTROL_CMD			3
#define DATA_REPOART		4
#define ERROR				5*/



// int socket_server_bussiness(void);
int socket_bussiness(void);
// int socket_ser2net_bussiness(void);
void doit(char *receivedData, int dataLength);
jsonType judgeJsonType(char * receivedData, int dataLength);
void sigalrm_read_ad_value(int sig);
void init_sigaction(void);
void init_time(long ms);
void doControlInfo(char *receivedData);
void read_and_do_with_fifo_data(SOCKET_INTERFACE *socket_interface, fd_set inputs);
int check_socket_connection(SOCKET_INTERFACE *socket_interface, fd_set *inputs);   //返回0，正常连接，-1，无连接
void oxygenFlowReport(char *receivedData);


#endif