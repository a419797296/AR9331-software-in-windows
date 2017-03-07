#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gpio.h"
#include "main.h"
#include "socket_server.h"
#include "ad7606.h"
#include "cJSON.h"
#include "ar9331_spi.h"
#include <fcntl.h>
#include <errno.h> 

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>

#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PERM S_IRUSR|S_IWUSR  //定义共享内存模式
#define DEV_MAX 5
char devname[DEV_MAX][10];
int valuedDev=0;
int shmid; 
char *ser2net_addr,*cloud_addr;  //共享内存映射地址
int devshmid; 
char *p_dev_addr,*c_dev_addr;  //共享内存映射地址

int ser2net_pid,cloud_pid;       //串口转网络&云端客户端进程号
short int s_adc_now[8];			 //保存ad7606采样结果
int ser2net_socket_id,cloud_socket_id;	//串口转网络&云端客户端socket通道号			 
unsigned char ad_result[SAMPLE_NUM];  //ad7606采集结果缓存数据


FILE * CV_AB_FILE, *CLK_FILE,*BUSY_FILE,*MISO_FILE;


//--------------------------判断是否是json格式---------
enum jsonType judgeJsonType(char * receivedData)
{
	cJSON *json;
	static char *json_type;
	json=cJSON_Parse(receivedData);
	if (!json) 
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
	}
	else
	{
		json_type = cJSON_GetObjectItem(json,"jsonType")->valuestring;
		if (strcmp(json_type,"controlInfo")==0)
		{
			printf("该Json串是controlInfo\r\n");
			return controlInfo;

		}
		else if (strcmp(json_type,"wifiInfo")==0)
		{
			printf("该Json串是wifiInfo\r\n");
			return wifiInfo;
		}
		else if (strcmp(json_type,"dataReport")==0)
		{
			printf("该Json串是dataReport\r\n");
			return dataReport;
		}
		else
		{
			printf("无法解析该json串\r\n");
		}
		cJSON_Delete(json);
	}
	return chatInfo;
}



/* Parse text to JSON, then render back to text, and print! */
void doit(char *receivedData,int sockfd)
{
	
	enum jsonType jsontype;
	jsontype=judgeJsonType(receivedData);
	switch(jsontype)
	{
		case chatInfo:
			printf("11111111111111\r\n");
			break;
		case controlInfo:
			printf("2222222222222\r\n");
			socketWrite(sockfd,ad_result,doControlInfo(receivedData,ad_result)+2);
			break;
		case wifiInfo:
			printf("3333333333333\r\n");
			break;
		case dataReport:
			printf("44444444444444\r\n");
			dataReportFork(receivedData,ad_result);
			break;
		default:
			printf("55555555555555\r\n");
			break;
	}
}

/* Read a file, parse, render back, etc. */
void dofile(char *filename)
{
	FILE *f;long len;char *data;
	int sockfd=0;
	f=fopen(filename,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
	doit(data,sockfd);
	free(data);
}



//----------------------------------服务器模式------
void scoketServerFork()
{
	int sockfd,new_fd;
	int client_num=0;
	int nbytes;
	static char readbuff[200];	
	sockfd = socketInit(SOCKET_PORT);
	// new_fd = socketAccept(sockfd);
	while(1)
	{
		new_fd = socketAccept(sockfd);
		client_num++;
		printf("第%d个客户端已成功连接！\n", client_num);
		if(fork()==0)
   		{
        	/* 子进程处理客户端的连接 */
        	while(1)
        	{
				if((nbytes=read(new_fd,readbuff,200))<=0) 
				{ 
					fprintf(stderr,"Read Error:%s\n",strerror(errno)); 
					close(sockfd);
					printf("第%d个客户端已经下线\n",client_num);  
					client_num--;                        //需要进程间通讯
					break;
				} 	
				else
				{
					readbuff[nbytes]='\0';
					printf("received data is: %s\n",readbuff);
					doit(readbuff,new_fd);
        	}
				}	

        	
   		}
	}
}
//----------------------------------客户端模式------
void scoketClientFork()
{
	int nbytes;
	static char readbuff[200];	

    /* 使用hostname查询host 名字 */
	cloud_socket_id=socketConnect(SOCKET_CLOUD_SERVER_IP,SOCKET_PORT);
	sendProductInfo(cloud_socket_id,"00:CA:01:0F:00:01");
	printf("the socket num is: %d\n",cloud_socket_id);
	//if (cloud_socket_id==0)           //链接成功
	{
		//socketWrite(cloud_socket_id,ad_result,2);
		printf("数据长度为%d\n",strlen(readbuff)); 
		while(1)
	    {
			if((nbytes=read(cloud_socket_id,readbuff,200))<=0) 
			{ 
				fprintf(stderr,"Read Error:%s\n",strerror(errno)); 
				printf("%d, %d\n",nbytes,readbuff[0]);
				// close(cloud_socket_id);
				break; 
			} 	
			else
			{
				printf("nbytes is : %d, readbuff[0] is :%d\n",nbytes,readbuff[0]);
				readbuff[nbytes]='\0';
				printf("received data is: %s\n",readbuff);
				doit(readbuff,cloud_socket_id);
				sleep(1);
			}	

	    }
	}
	
}

//----------------------------------客户端模式------
void serToNetFork()
{
	int nbytes;
	static char readbuff[1024];	

    /* 使用hostname查询host 名字 */
	ser2net_socket_id=socketConnect(SOCKET_LOCAL_SERVER_IP,SOCKET_PORT);
	printf("serial to net server have been started\n");
	//if (ser2net_socket_id==0)           //链接成功
	{
		//socketWrite(ser2net_socket_id,ad_result,2);
		printf("数据长度为%d\n",strlen(readbuff)); 
		while(1)
	    {
			if((nbytes=read(ser2net_socket_id,readbuff,1024))<=0) 
			{ 
				fprintf(stderr,"Read Error:%s\n",strerror(errno)); 
				printf("%d, %d\n",nbytes,readbuff[0]);
				// close(ser2net_socket_id);
				break; 
			} 	
			else
			{
				printf("nbytes is : %d, readbuff[0] is :%d\n",nbytes,readbuff[0]);
				readbuff[nbytes]='\0';
				printf("received data is: %s\n",readbuff);
				
				memset(ser2net_addr,'\0',1024); 
				//printf("ser2net_addr data is0: %s;cloud_addr data is:%s; strlen(cloud_addr) is %d\n",ser2net_addr,cloud_addr,strlen(cloud_addr));
				strncpy(ser2net_addr,readbuff,1024);
				printf("ser2net_addr data is0: %s;cloud_addr data is:%s;\n",ser2net_addr,cloud_addr);
				//socketWrite(cloud_socket_id,(unsigned char *)cloud_addr,strlen(cloud_addr));
				kill(cloud_pid,SIGUSR1);
					// doit(readbuff,ser2net_socket_id);
				// sleep(1);
			}	

	    }
	}
	
}


void sendProductInfo(int sockfd,char * macAddr)
{
	cJSON *root;
	char *out;
	// char outcmd[100];
	// char *startRecvDateCmd;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root, "jsonType", "productInfo");
	cJSON_AddStringToObject(root, "productMac", macAddr);
	out=cJSON_PrintUnformatted(root);
	// sprintf(outcmd,"%s",out);	
	 printf("%d\n",strlen(out));		
	socketWrite(sockfd,(unsigned char *)out,strlen(out));
	sleep(1);
	cJSON_Delete(root);	
	free(out);


}
int doControlInfo(char *receivedData,unsigned char *ad_result)
{
	cJSON *json;
	int freq,sampleNum;
	// char *userName;
	// char *out;

	memset(ad_result,'\0',SAMPLE_NUM); 
	//------------------获取参数----
	json=cJSON_Parse(receivedData);
	freq = cJSON_GetObjectItem(json,"freq")->valueint;
	sampleNum = cJSON_GetObjectItem(json,"sampleNum")->valueint;
	// userName = cJSON_GetObjectItem(json,"userName") ->valuestring;

	// printf("the freq is: %d, the sample nums is %d\r\n",freq,sampleNum);
	// out=cJSON_Print(json);
	// cJSON_Delete(json);
	// printf("%s\n",out);
	// free(out);

	//-------------------------向服务器发送开始采集信号-------------------
	//startRecvDate(cloud_socket_id,freq,sampleNum, userName);

	//------------------采集发送传感器数据----
	strcpy((char *)ad_result,receivedData);
	printf("%s\n", ad_result);
	strcat((char *)ad_result,SPLITER);
	printf("%s\n", ad_result);
	struct timeval tpstart,tpend;
	float timeuse;

	//if (freq!=NULL&&sampleNum!=NULL)
	{


		gettimeofday(&tpstart,NULL);//开始时间
		printf("the sizeof(SPLITER) is %d\n", sizeof(SPLITER));
		readAD(freq,sampleNum,ad_result+strlen(receivedData)+sizeof(SPLITER)-1); //3表示666的分隔符

		gettimeofday(&tpend,NULL);//开始时间

		//--------------计算程序用时
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
		timeuse/=1000000;
		printf("Used Time: %f\n", timeuse);

	}
	printf("%s\n", ad_result);
	printf("%d\n", strlen((char *)ad_result));
	return strlen((char *)ad_result);
}

void dataReportFork(char *receivedData,unsigned char *ad_result)
{
	cJSON *json;
	int timeInterval;
	char *devName;
	

		// char *out;
	//------------------获取参数----
	json=cJSON_Parse(receivedData);
	timeInterval = cJSON_GetObjectItem(json,"timeInterval")->valueint;
	devName = cJSON_GetObjectItem(json,"devName")->valuestring;
	printf("timeInterval: %d,devName:%s\n", timeInterval,devName);

	*p_dev_addr=timeInterval;

	if(!findTheSameDev(devName))//如果没找到
	{
		printf("未给改设备分配独立进程\n");
		strcpy((char *)ad_result,receivedData);
		strcat((char *)ad_result,SPLITER);
		if (fork()==0)
		{
			
			c_dev_addr=shmat(devshmid,0,0);

			printf("收到的timeInterval is %d\n", *c_dev_addr);
			while (*c_dev_addr)    //zero means stop monitoring
			{
				printf("the monitoring is running\n");
				reportAD(devName,*c_dev_addr,ad_result+strlen(receivedData)+sizeof(SPLITER)-1);
				socketWrite(cloud_socket_id,(unsigned char *)ad_result,strlen((char *)ad_result)+2);
			}
			exit(0);
		}
		else
		{
			;
		}

	}

}

int findTheSameDev(char *name)
{
	int i;
	for (i = 0; i <DEV_MAX ; ++i)
	{
		if (!strcmp(devname[i],name))
			return 1;
	}
	printf("未找到相应的设备\n");
	strcpy(devname[valuedDev++],name);
	return 0;
}

void signal_func(int sign_no)
{
	if(sign_no==SIGUSR1)
	{
		printf("the share Memory is %s\n", cloud_addr);
		socketWrite(cloud_socket_id,(unsigned char *)cloud_addr,strlen(cloud_addr));
	}

}
void killChildPid(int sign_no)
{

	kill(cloud_pid,SIGINT);
	kill(ser2net_pid,SIGINT);
	printf("killing the process...");
	exit(0);

}

//----------------------------------主函数-----------
int main(int argc, char **argv)
{
	/* code */
	
	int c;
	char *msg;
	// int value;


	while ((c = getopt(argc, argv, "hrm:")) != EOF) {
		switch (c) {
		case 'm':
			msg = optarg;
			if (strcmp(msg,"spi")==0||strcmp(msg,"gpio")==0)
				ad7606_Init(msg);
			else
				goto usage;
			continue;
		case 'r':	
			gpioRelease(numList);
			return 1;
		case 'h':
		case '?':
usage:
			fprintf(stderr,
				"usage: %s [-h] [-m spi/gpio]\n",
				argv[0]);
			return 1;
		}
	}

	if (optind != argc)
	{
		printf("%d, %d\n", optind + 1,argc);
		goto usage;		
	}

	signal(SIGINT, killChildPid);

	/* 创建共享内存 */	
	if((shmid=shmget(IPC_PRIVATE,1024,PERM))==-1) 
	{ 
		fprintf(stderr,"Create Share Memory Error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 
	 	/* 创建共享内存 */	
	if((devshmid=shmget(IPC_PRIVATE,1024,PERM))==-1) 
	{ 
		fprintf(stderr,"Create Share Memory Error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 



	if((cloud_pid=fork())==0)   //子进程进入客户端模式
	{
		cloud_addr=shmat(shmid,0,0);
		p_dev_addr=shmat(devshmid,0,0); 
		memset(p_dev_addr,'\0',1024); 
		printf("the cloud_addr is %d\n", (int)cloud_addr);
		signal(SIGUSR1, signal_func);
		signal(SIGINT, SIG_DFL);
		scoketClientFork();
	}
	else		//父进程
	{
		if ((ser2net_pid=fork())==0)
		{
			signal(SIGINT, SIG_DFL);
			ser2net_addr=shmat(shmid,0,0); 
			printf("the ser2net_addr is %d\n", (int)ser2net_addr);
			serToNetFork();//子进程运行ser2net程序
		}
		else
		{
			printf("the ser2net_pid is %d\n", ser2net_pid);
			printf("the cloud_pid is %d\n", cloud_pid);
			printf("the parent pid is %d\n", getpid());
			//scoketServerFork(); //父进程运行服务器模式
			while(1)
			{
				sleep(1);
			}

		}
	}

	return 0;
}
