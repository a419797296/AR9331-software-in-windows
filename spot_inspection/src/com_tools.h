#ifndef COM_TOOLS_H_
#define COM_TOOLS_H_
int getMacAddr(char *device,char * macAddrBuff);
void sendProductInfo(int sockfd,char * macAddr);		 
int SendStringSCIByPackage(int socket_fd,char * dataString,int packageSize,int delayTime); 
int  JsonResolveInt(char* dataString, char *str);
void StrToHex(char *pbDest, char *pbSrc, int nLen);
void HexToStr(char *pbDest, char *pbSrc, int nLen);
int pow_of_two(int num);
#endif