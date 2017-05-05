#ifndef COM_TOOLS_H_
#define COM_TOOLS_H_
#include "main.h"

extern int getMacAddr(char *device,char * macAddrBuff);
extern void sendProductInfo(int sockfd);
extern void sendAlarmInfo(int sockfd, sysAlarm sys_alarm);
extern int SendStringSCIByPackage(int socket_fd,char * dataString,int packageSize,int delayTime); 
extern int  JsonResolveInt(char* dataString, char *str);
extern void StrToHex(char *pbDest, char *pbSrc, int nLen);
extern void HexToStr(char *pbDest, char *pbSrc, int nLen);
extern int pow_of_two(int num);
extern char * getSysUciCfgStr(char *filename,char *section,char *option,char * result);
extern int getSysUciCfgNum(char *filename,char *section,char *option);
extern int setSysUciCfgStr(char *filename,char *section,char *option,char * parameter);
extern int setSysUciCfgNum(char *filename,char *section,char *option,int parameter);
extern int calc_system_soc(void);
extern int calc_shift_virb(void);
extern char XorCheckSum(char * pBuf, char len);
extern int getDataPkgFromSerial(char *conbined_buf, int *conbined_len, char *new_data, int new_data_len, char first_byte, char end_byte, int max_pkg_len) ; 
#endif