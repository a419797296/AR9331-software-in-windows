#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>   //用户小写字符转大写  
#include "socket_driver.h"
#include "socket_client.h"
#include "socket_server.h"
#include "com_tools.h"
#include "main.h"
#include "cJSON.h"

//-----------------------------------------------------
int getMacAddr(char *device,char * macAddrBuff)
{
    FILE *read_fp;
    int chars_read;
    int ret;
    char cmd_buff[60]={0};
    sprintf(cmd_buff,"ifconfig %s|grep 'HWaddr'|awk -F '[ ]+' '{print $5}'",device);
    // sprintf(cmd_buff,"ifconfig %s|grep 'HWaddr'|awk -F '[ ]+' '{print $5}|tr -d '\n'",device);
    read_fp=popen(cmd_buff,"r");

    if (read_fp!=NULL)
    {
        chars_read=fread(macAddrBuff,sizeof(char),20,read_fp);
        *(macAddrBuff+17) = 0;   //the length of macAddr is 17
        printf("the macAddr is %s\n", macAddrBuff);
        if (chars_read>0)
        {
            ret=1;/* code */
        }
        else
        {
            ret=-1;
        }
        pclose(read_fp);
    }
    else
    {
        ret=-1;
    }
    return ret;
}

//-----------------------------------------------------
void sendProductInfo(int sockfd,char * macAddr)
{
	cJSON *root;
	char *out;
	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "jsonType", JSON_TYPE_PRODUCT_INFO);
	cJSON_AddStringToObject(root, "productMac", macAddr);
    cJSON_AddStringToObject(root, "hardware", HARDWARE_VERSION);
    cJSON_AddStringToObject(root, "software", SOFTWARE_VERSION);
	out=cJSON_PrintUnformatted(root);
	// sprintf(outcmd,"%s",out);	
	 // printf("%d\n",strlen(out));		
	socketWriteNoEnd(sockfd,out,strlen(out)+1);
	cJSON_Delete(root);	
	free(out);
}
//*****************************************************************************
//函数名：SendStringSCIByPackage
//参  数：无
//返回值：无
//功  能：分包发送字符串
//说  明：
//日  期：2016年6月15日1:25:17
//***************************************************************************** 
 int SendStringSCIByPackage(int socket_fd,char * dataString,int packageSize,int delayTime) 
 {
    int packageNum=0;
    char packageTmp[packageSize];
    int i=0;
    memset(packageTmp,0,packageSize);        //32*sizeof(int)
    while(*(dataString+(packageNum<<5)+i)!='\0') {
      if(i!=packageSize) {
         *(packageTmp+i)=*(dataString+(packageNum<<5)+i); 
      }else {
        i=0;
        packageNum++;
        socketWriteNoEnd(socket_fd,packageTmp,packageSize);
        usleep(delayTime);
        memset(packageTmp,0,packageSize);        //32*sizeof(int)
        *(packageTmp)=*(dataString+(packageNum<<5));
      }
      i++;  
    }
    
    socketWriteNoEnd(socket_fd,packageTmp,i); 
    return (packageNum<<5)+i;   
}

int JsonResolveInt(char* dataString, char *str)
{
    char *index;
    char *index1;
    char *res;
    char key[20];
    char value[5];
    //printf("%s\n", dataString);
    sprintf(key,"%s\":",str);
    //printf("%s\n", key);
        if (NULL != (index = strstr(dataString, key))){
            res = index + strlen(key);
            if (NULL != (index1 = strstr(res,"}"))){
                memset(value, 0, 5);
                strncpy(value, res, index1-res);
                return atoi(value);
            }

        }  
    return -1;  
}

// C语言版的16进制与字符串互转函数
/*
// C prototype : void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
//  [IN] pbSrc - 字符串
//  [IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value: 
// remarks : 将字符串转化为16进制数
*/
void StrToHex(char *pbDest, char *pbSrc, int nLen)
{
    char h1,h2;
    char s1,s2;
    int i;

    for (i=0; i<nLen; i++)
    {
        h1 = pbSrc[(i<<1)];
        h2 = pbSrc[(i<<1)+1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9) 
        s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9) 
        s2 -= 7;

        pbDest[i] = (s1<<4) + s2;
    }
}

/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
//  [IN] pbSrc - 输入16进制数的起始地址
//  [IN] nLen - 16进制数的字节数
// return value: 
// remarks : 将16进制数转化为字符串
*/
void HexToStr(char *pbDest, char *pbSrc, int nLen)
{
    char    ddl,ddh;
    int i;

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + ((unsigned char)pbSrc[i] >> 4);
        ddl = 48 + ((unsigned char)pbSrc[i] % 16);
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[(i << 1)] = ddh;
        pbDest[(i << 1)+1] = ddl;
    }

    pbDest[(nLen <<1)] = '\0';
}

/*
//求出一个数对于2的幂指数
*/
int pow_of_two(int num)
{
    int pow_num = 0;
    if (num != 0)
    {
        while((num>>=1)!=0)
            pow_num++;
    }
    else
    {
        pow_num = -1;
    }

    // printf("%d\n", pow_num);
    return pow_num + 1;
}