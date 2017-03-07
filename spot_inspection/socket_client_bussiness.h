#ifndef SOCKET_CLIENT_BUSSINESS
#define SOCKET_CLIENT_BUSSINESS

#define MAX_LENGTH_BY_ONCE			1000
#define SHIFT_BIT_TO_PACKAGE_LENGTH				6
#define PACKAGE_LENGTH						64


enum jsonType{chatInfo,wifiInfo, controlInfo,dataReport};
// int socket_server_bussiness(void);
int socket_bussiness(void);
// int socket_ser2net_bussiness(void);
void doit(char *receivedData, int fd);
enum jsonType judgeJsonType(char * receivedData);
void sigalrm_read_ad_value(int sig);
void init_sigaction(void);
void init_time(long ms);
void doDataReport(char *receivedData);
void doControlInfo(char *receivedData, int fd);
void read_and_do_with_fifo_data(SOCKET_INTERFACE *socket_interface, fd_set inputs);
int check_socket_connection(SOCKET_INTERFACE *socket_interface, fd_set *inputs);   //返回0，正常连接，-1，无连接

void StrToHex(char *pbDest, char *pbSrc, int nLen);
void HexToStr(char *pbDest, char *pbSrc, int nLen);

#endif