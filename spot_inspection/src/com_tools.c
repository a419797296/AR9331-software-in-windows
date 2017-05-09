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
#include "ad7606_app.h"
//-----------------------------------------------------
int getMacAddr(char *device,char * macAddrBuff)
{
    FILE *read_fp;
    int chars_read;
    int ret = -1;
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
            ret = 0;/* code */
        }
        else
        {
            ret = -1;
        }
        pclose(read_fp);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

//-----------------------------------------------------
void sendProductInfo(int sockfd)
{
	cJSON *root;
	char *out;

	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "jsonType", JSON_TYPE_PRODUCT_INFO);
	cJSON_AddStringToObject(root, "productMac", produc_info.mac);
    cJSON_AddStringToObject(root, "hardware", produc_info.hw_vers);
    cJSON_AddStringToObject(root, "software", produc_info.sw_vers);
	out=cJSON_PrintUnformatted(root);
	// sprintf(outcmd,"%s",out);	
	 // printf("%d\n",strlen(out));	
	socketWriteNoEnd(sockfd,out,strlen(out)+1);
	cJSON_Delete(root);	
	free(out);
}

//-----------------------------------------------------
void sendAlarmInfo(int sockfd, sysAlarm sys_alarm)
{
	cJSON *root;
	char *out;
	if(sockfd<=0)
		return;
	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "jsonType", JSON_TYPE_SYS_ALARM);
	cJSON_AddNumberToObject(root, "alarmNum", sys_alarm);
	cJSON_AddStringToObject(root, "productMac", produc_info.mac);
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

//-----------------------------------------------------
char * getSysUciCfgStr(char *filename,char *section,char *option,char * result)
{
    FILE *read_fp;
    int chars_read;
	char *ret;
    char cmd_buff[60]={0};
    sprintf(cmd_buff,"uci get %s.%s.%s",filename,section,option);
    // sprintf(cmd_buff,"ifconfig %s|grep 'HWaddr'|awk -F '[ ]+' '{print $5}|tr -d '\n'",device);
    read_fp=popen(cmd_buff,"r");

    if (read_fp!=NULL)
    {
        chars_read=fread(result,sizeof(char),60,read_fp);

        if (chars_read>0)
        {
            *(result+chars_read-1) = 0;   // the read result include '\n'
            printf("the read config is %s\n", result);
		ret =  result;
        }
        else
        {
            ret = NULL;
        }
        pclose(read_fp);
    }
    else
    {
        ret = NULL;
    }
    return ret;
}
//-----------------------------------------------------
int getSysUciCfgNum(char *filename,char *section,char *option)
{
    FILE *read_fp;
    int chars_read;
	int ret;
    char cmd_buff[60]={0};
	char result[10] = {0};
    sprintf(cmd_buff,"uci get %s.%s.%s",filename,section,option);
    // sprintf(cmd_buff,"ifconfig %s|grep 'HWaddr'|awk -F '[ ]+' '{print $5}|tr -d '\n'",device);
    read_fp=popen(cmd_buff,"r");

    if (read_fp!=NULL)
    {
        chars_read=fread(result,sizeof(char),60,read_fp);

        if (chars_read>0)
        {
		ret = atoi(result);
            printf("the read config is %d\n", ret );
        }
        else
        {
            ret = -1;
        }
        pclose(read_fp);
    }
    else
    {
        ret = -1;
    }
    return ret;
}
//-----------------------------------------------------
int setSysUciCfgStr(char *filename,char *section,char *option,char * parameter)
{
    char cmd_buff[60]={0};
	int rst;
	sprintf(cmd_buff,"uci set %s.%s.%s=%s",filename,section,option,parameter);
	rst = system(cmd_buff);
	system("uci commit");
    return rst;
}

//-----------------------------------------------------
int setSysUciCfgNum(char *filename,char *section,char *option,int parameter)
{
    char cmd_buff[60]={0};
	int rst;
    sprintf(cmd_buff,"uci set %s.%s.%s=%d",filename,section,option,parameter);
	rst = system(cmd_buff);
	system("uci commit");
    return rst;
}

//-----------------------------------------------
int calc_system_soc(void)
{
    int system_soc=0;
    set_acqusition_para(10, 1, 1, "2");
    channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, acqusition_para.length);		// free(channel_info);
    acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
    system_soc = *(channel_info->data);
    //PLOG("the system_soc is %d\n",system_soc);
    free(channel_info);
    if(system_soc > 4100)
        system_soc = 4100;
    if(system_soc < 3300)
        system_soc = 3300;
    system_soc = (system_soc - 3300) / 8;
    return system_soc;
}

int calc_shift_virb(void)
{
    int virb_shitf=0, sum = 0;
	int Nums = 100;
	int i;
    set_acqusition_para(100, Nums, 1, "0");
    channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, acqusition_para.length);		// free(channel_info);
    acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
	for(i=0;i<Nums;i++)
		{
		sum +=*(channel_info->data+i);
	}
	virb_shitf =sum / Nums;
	if(((sum % Nums)<<1) > virb_shitf)    //four dropped and five accepted
		virb_shitf ++;
    return virb_shitf;
}
//-----------------------calulate the xor-------------------------------
char XorCheckSum(char * pBuf, char len)
{
  int i;
  char byRet=0;
  
  if(len == 0)
    return byRet;
  else
    byRet = pBuf[0];
  
  for(i = 1; i < len; i ++)
    byRet = byRet ^ pBuf[i];
  
  return byRet;
}
//-----------------------------------------------------
int getDataPkgFromSerial(char *conbined_buf, int *conbined_len, char *new_data, int new_data_len, char first_byte, char end_byte, int max_pkg_len)  //if end_byte ==0.means if recieved 0,then return;end_byte ==0xff, ignore the end byte;else,must match the end byte. 
{
	PLOG("------%s,data len is %d,------\n",new_data,*conbined_len);
	memcpy(conbined_buf + *conbined_len, new_data, new_data_len);
	*conbined_len=*conbined_len+new_data_len;
	if(end_byte=='\0')
		{
		if(new_data[new_data_len-1]!=end_byte)
			{
			if(*conbined_len>max_pkg_len)
				*conbined_len=0;
			return 0;
		}
		else
			{
			return 1;
		}

	}
	else
		{
		if(*conbined_len==0)
			{
			if(first_byte!='\0')
				{
				if(new_data[0]!=first_byte)
					{
					*conbined_len=0;
					PLOG("can not match the first byte");
					return 0;
				}
					
			}
		}
		char xor;
		int data_len,pkg_len;
		data_len=*(conbined_buf+4);
		pkg_len=data_len+8;

		if(*conbined_len>=max_pkg_len||*conbined_len>=pkg_len)
			{
			
			if(end_byte!=0xff)
				{
				if(new_data[new_data_len-1]!=end_byte)
					{
					PLOG("can not match the end byte");
					*conbined_len=0;
					return 0;
				}
			}
					

			xor=XorCheckSum(conbined_buf+2,data_len+3); //add the 2 byte of addr, 1 byte of len, so in total is 3bytes
			if(xor==*(conbined_buf+data_len+5))   //24 40 00 00 len data xor
				return 1;					//here is packaged, return 0 means OK;
			else
				{
				PLOG("can not match the xor byte");
				*conbined_len=0;
				return 0;
			}
				
				
		}
		else
			return 0; //not packaged, then return		
	}
	return 0; //not packaged, then return	

}