#ifndef MAIN_H_
#define MAIN_H_

#define SAMPLE_NUM 10000
#define SOCKET_PORT 3333
#define SOCKET_LOCAL_SERVER_IP "127.0.0.1"
#define SOCKET_CLOUD_SERVER_IP "192.168.31.133"
#define SPLITER "WILLTECH_BLUE"

enum jsonType{chatInfo,wifiInfo, controlInfo,dataReport};

void doit(char *receivedData,int sockfd);
void dofile(char *filename);
int doControlInfo(char *receivedData,unsigned char *ad_result);
void scoketClientFork();
void scoketServerFork();
enum jsonType judgeJsonType(char * receivedData);
void startRecvDate(int sockfd,int freq, int sampleNum, char * userName);
void sendProductInfo(int sockfd,char * macAddr);
void serToNetFork();
void signal_func(int sign_no);
void dataReportFork(char *receivedData,unsigned char *ad_result);
int findTheSameDev(char *name);

#endif