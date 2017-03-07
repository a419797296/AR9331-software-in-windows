#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_


int socketInit(int portnumber);
int socketAccept(int sockfd);
int socketRead(int new_fd,char *readbuff,int lenth);
int socketWrite(int new_fd,unsigned char *writebuff,int lenth);
void socketClose(int sockfd,int new_fd);
int socketConnect(char * ip,int portnumber);
#endif

