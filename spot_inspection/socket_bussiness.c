#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "socket_driver.h"
#include "socket_client.h"
#include "socket_server.h"
#include "ser2net.h"
#include "socket_bussiness.h"
#include "ad7606_app.h"
#include "com_tools.h"
#include "cJSON.h"
#include "main.h"

int socket_bussiness(void)
{


    int result;
    fd_set inputs;
    uint8 soc=0;
    struct timeval timeout;
    PLOG("Preparing for reading bytes...\n");
    while(1)
    {
        FD_ZERO(&inputs);//用select函数之前先把集合清零
        //FD_SET(tty_interface.fifo_rd_fd,&inputs);//加入到集合里


        if (enable_mode[SOCKET_SER2NET_NUM] == '1')
        {
            if (check_socket_connection(&socket_ser2net_interface, &inputs) < 0)
            {
                if(connect_flag[SOCKET_SER2NET_NUM])
                {
                    connect_flag[SOCKET_SER2NET_NUM] = 0;
                    system("/root/led.sh blink_slow tp-link:blue:system");
                }

                serToNetFork();

            }
        }

        if (enable_mode[SOCKET_SERVER_NUM] == '1')
        {
            if (check_socket_connection(&socket_server_interface, &inputs) < 0)
            {
                if(connect_flag[SOCKET_SERVER_NUM])
                {
                    system("/root/led.sh blink_slow tp-link:blue:system");
                    connect_flag[SOCKET_SERVER_NUM] = 0;
                }

                socketServerFork(socket_server_interface.port);

            }
        }

        if (enable_mode[SOCKET_CLIENT_NUM] == '1')
        {
            if (check_socket_connection(&socket_client_interface, &inputs) < 0)
            {
                if(connect_flag[SOCKET_CLIENT_NUM])
                {
                    system("/root/led.sh blink_slow tp-link:blue:system");
                    connect_flag[SOCKET_CLIENT_NUM] = 0;
                }

                socketClientFork(socket_client_interface.ip, socket_client_interface.port);

            }
        }

        timeout.tv_sec = 2;
        timeout.tv_usec = 500000;
        result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, &timeout);
        switch(result)
        {
        case 0:
            // PLOG("timeout\n");
            break;
        case -1:
            perror("select");
            exit(1);
        default:

            read_and_do_with_fifo_data(&socket_ser2net_interface, inputs);
            read_and_do_with_fifo_data(&socket_client_interface, inputs);
            //read_and_do_with_fifo_data(&tty_interface, inputs);
            read_and_do_with_fifo_data(&socket_server_interface, inputs);
            break;

        }
        soc = calc_system_soc();
        if(soc < LOW_POWER_ALARM_LEVEL)
        {
            system("/root/led.sh blink_fail tp-link:blue:system");
        }
        PLOG("-------------the  system_soc is %d%%-----------\n",soc);
    }
    pause(); /*暂停，等待信号*/
    unlink(SOCKET_CLIENT_FIFO_PATH); //删除文件
    unlink(SOCKET_SERVER_FIFO_PATH); //删除文件
    unlink(SOCKET_SER2NET_FIFO_PATH); //删除文件
    unlink(TTY_FIFO_PATH); //删除文件

}


//------------------------------------------------------read_and_do_with_fifo_data
void read_and_do_with_fifo_data(SOCKET_INTERFACE *socket_interface, fd_set inputs)
{
    int  nread;
    char buf_r[200];
    if(FD_ISSET((*socket_interface).fifo_rd_fd, &inputs))   //判断当前接口的文件描述符是否属于监测的集合
    {
        nread=read((*socket_interface).fifo_rd_fd,buf_r,sizeof(buf_r));
        buf_r[nread] = 0;
        PLOG("read %s from %s\n",buf_r,socket_interface->fifo_path);
        doit(buf_r, nread);
    }
}

//------------------------------------------------------read_and_do_with_fifo_data
int check_socket_connection(SOCKET_INTERFACE *socket_interface, fd_set *inputs)   //返回0，正常连接，-1，无连接
{
    if(SocketConnected((*socket_interface).socket_fd))
    {
        // PLOG("socket is connected\n");
        FD_SET((*socket_interface).fifo_rd_fd,inputs);//加入到集合里
        return 0;
    }
    else
    {
        PLOG("(*socket_interface) is unconnected\n");
        close((*socket_interface).socket_fd);
        return -1;
    }
}



/* Parse text to JSON, then render back to text, and print! */
void doit(char *receivedData, int dataLength)
{

    jsonType jsontype;
    jsontype=judgeJsonType(receivedData,dataLength);
    switch(jsontype)
    {
    case JSON_TYPE_ERROR:
        PLOG("jsonType is ERROR\r\n");
        break;
    case JSON_TYPE_CONTROL_CMD:
        PLOG("jsonType is CONTROL_CMD\r\n");
        doControlInfo(receivedData);
        break;
    case JSON_TYPE_GETWAY_TO_ZIGBEE:
        PLOG("jsonType is GETWAY_TO_ZIGBEE\r\n");
        socketWriteByPackages(socket_ser2net_interface.socket_fd,receivedData, dataLength, 64, 20000);
        break;
    case JSON_TYPE_ZIGBEE_TO_GETWAY:
        PLOG("jsonType is ZIGBEE_TO_GETWAY\r\n");
        *(receivedData+dataLength)=0;
        if (*(enable_mode+1)=='1')//if the app works in server model, then send the data back to client
        {
            socketWriteNoEnd(socket_server_interface.socket_fd, receivedData, dataLength+1);   //+1 means sent '/0' together
            printf("%c\n", *(enable_mode+1));
        }

        if (*(enable_mode+2)=='1')//if the app works in client model, then send the data back to server
            socketWriteNoEnd(socket_client_interface.socket_fd, receivedData, dataLength+1);   //+1 means sent '/0' together
        break;
    case JSON_TYPE_DATA_REPOART:
        PLOG("jsonType is DATA_REPOART\r\n");
        doDataReport(receivedData);
        break;
    case JSON_TYPE_OXYGEN:
        PLOG("jsonType is JSON_TYPE_OXYGEN\r\n");
        oxygenFlowReport(receivedData);
        break;
    default:
        PLOG("jsonType is default+\r\n");
        break;
    }
}
//--------------------------判断是否是json格式---------
jsonType judgeJsonType(char * receivedData, int dataLength)
{
    cJSON *json;
    static jsonType json_type;
    if(*receivedData=='{')
    {
        json=cJSON_Parse(receivedData);
        if (!json)
        {
            PLOG("Error before: [%s]\n",cJSON_GetErrorPtr());
        }
        else
        {
            json_type = cJSON_GetObjectItem(json,"jsonType")->valueint;
            cJSON_Delete(json);
        }
    }
    else if(*receivedData==0x16)
        json_type = JSON_TYPE_OXYGEN;
    else
    {
        if (receivedData[0]=='$' && receivedData[1]=='@' && receivedData[dataLength-2]=='\r' && receivedData[dataLength-1]=='\n')
        {
            printf("------------------success+++success--%d------------\n",dataLength);
            json_type = JSON_TYPE_GETWAY_TO_ZIGBEE;
        }
        else
        {
            printf("------------------ERROR+++success--%d------------\n",dataLength);
            json_type = JSON_TYPE_ERROR;
        }
    }
    return json_type;
}
void doControlInfo(char *receivedData)
{
    cJSON *json, *channel_json;
    int data_packages, data_residual_length;
    int freq,length;
    char *channels, channel_nums;
    char channel_list[8]= {0};
    int i,j;
    char *out, *disp;
    int is_exist_channel_info = 0;
    // char *end_flag = "\0";
    PLOG("receivedData = %s\n",receivedData);


    //------------------获取参数----
    json=cJSON_Parse(receivedData);
    freq = cJSON_GetObjectItem(json,"freq")->valueint;
    length = cJSON_GetObjectItem(json,"sampleNum")->valueint;
    channels = cJSON_GetObjectItem(json,"channelList")->valuestring;



    channel_nums = strlen(channels);
    strcpy(channel_list,channels);
    PLOG("freq = %d, channel_nums = %d, channel_list = %s, length = %d\n",freq,channel_nums,channel_list,length);



    //------------------------------------------------------------如果长度小于指定长度，则一次性完成采集
    if (length < MAX_LENGTH_BY_ONCE)
    {
        char data_hex2str[(length << 2) +1];//长度为采集个数两倍加1
        set_acqusition_para(freq, length, channel_nums, channel_list);
        channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, acqusition_para.length);		// free(channel_info);
        acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
        for (i = 0; i < channel_nums; ++i)
        {
            HexToStr(data_hex2str, (char *)(*(channel_info+i)).data, (length<<1));
            PLOG("%s\n",data_hex2str);

            if (! is_exist_channel_info)
            {
                is_exist_channel_info = 1;
                cJSON_AddItemToObject(json, "channel_info", channel_json=cJSON_CreateObject());
            }
            else
                cJSON_ReplaceItemInObject(json, "channel_info", channel_json=cJSON_CreateObject());

            cJSON_AddNumberToObject(channel_json,"num",		(*(channel_info+i)).num);
            cJSON_AddStringToObject(channel_json,"name",		"virb_sens0");
            cJSON_AddNumberToObject(channel_json,"cur_package",		0);
            cJSON_AddNumberToObject(channel_json,"total_package",		1);
            cJSON_AddStringToObject(channel_json,"data",		data_hex2str);

            out = cJSON_PrintUnformatted(json);
            disp = cJSON_Print(json);
            PLOG("%s\n",out);

            if (*(enable_mode+1)=='1')//if the app works in server model, then send the data back to client
                socketWriteNoEnd(socket_server_interface.socket_fd, out, strlen(out)+1);   //+1 means sent '/0' together
            if (*(enable_mode+2)=='1')//if the app works in client model, then send the data back to server
                socketWriteNoEnd(socket_client_interface.socket_fd, out, strlen(out)+1);   //+1 means sent '/0' together

            /*if (fd == tty_interface.socket_fd)
            {
            	PLOG("this the zigbee modle\n");
            	socketWriteByPackages(fd, out, 32, 25000);   //32bytes per package, and the time interval is 25ms
            }
            else
            {

            	if (MATLAB_TEST)
            		socketWriteNoEnd(socket_client_interface.socket_fd, (char *)(*(channel_info+i)).data, (length<<1));
            	else
            		socketWriteNoEnd(fd, out, strlen(out)+1);   //+1 means sent '/0' together
            }*/



            free(out);	/* Print to text, Delete the cJSON, print it, release the string. */
            free(disp);

            // socketWriteNoEnd(socket_client_interface.socket_fd, (char *)(*(channel_info+i)).data, (length<<1));
        }

    }
    else
    {
        //------------------------------------------------------------分包采集
        int PACKAGE_LENGTH,SHIFT_BIT_TO_PACKAGE_LENGTH;
        SHIFT_BIT_TO_PACKAGE_LENGTH = pow_of_two(freq / package_freq);
        PACKAGE_LENGTH = 1 << SHIFT_BIT_TO_PACKAGE_LENGTH;
        printf("PACKAGE_LENGTH = %d, SHIFT_BIT_TO_PACKAGE_LENGTH = %d\n", PACKAGE_LENGTH,SHIFT_BIT_TO_PACKAGE_LENGTH);
        char data_hex2str[(PACKAGE_LENGTH << 2) +1];//长度为采集个数四倍加1
        data_packages = length >> SHIFT_BIT_TO_PACKAGE_LENGTH;
        data_residual_length = length % PACKAGE_LENGTH;
        if (data_residual_length != 0) //如果分包后剩下的数据长度不为零，则总包数+1
        {
            data_packages++;
        }

        set_acqusition_para(freq, PACKAGE_LENGTH, channel_nums, channel_list);
        channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, PACKAGE_LENGTH);		// free(channel_info);

        for (j = 0; j < data_packages; ++j)
        {
            acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
            for (i = 0; i < channel_nums; ++i)
            {
                if ((data_residual_length != 0) && (j == data_packages - 1))  //如果分包后还有数据，而且是最后一包数据
                    HexToStr(data_hex2str, (char *)(*(channel_info+i)).data, (data_residual_length<<1));  //转换剩下的个数
                else
                    HexToStr(data_hex2str, (char *)(*(channel_info+i)).data, (PACKAGE_LENGTH<<1));

                if (! is_exist_channel_info)
                {
                    is_exist_channel_info = 1;
                    cJSON_AddItemToObject(json, "channel_info", channel_json=cJSON_CreateObject());
                }
                else
                    cJSON_ReplaceItemInObject(json, "channel_info", 	channel_json=cJSON_CreateObject());

                cJSON_AddNumberToObject(channel_json,"num",		(*(channel_info+i)).num);
                cJSON_AddStringToObject(channel_json,"name",		"virb_sens");
                cJSON_AddNumberToObject(channel_json,"cur_package",		j);
                cJSON_AddNumberToObject(channel_json,"total_package",		data_packages);
                cJSON_AddStringToObject(channel_json,"data",		data_hex2str);


                out = cJSON_PrintUnformatted(json);
                disp = cJSON_Print(json);
                PLOG("%s\n",disp);

                if (*(enable_mode+1)=='1')//if the app works in server model, then send the data back to client
                    socketWriteNoEnd(socket_server_interface.socket_fd, out, strlen(out)+1);   //+1 means sent '/0' together
                if (*(enable_mode+2)=='1')//if the app works in client model, then send the data back to server
                    socketWriteNoEnd(socket_client_interface.socket_fd, out, strlen(out)+1);   //+1 means sent '/0' together


                /*if (fd == tty_interface.socket_fd)
                	socketWriteByPackages(fd, out, 32, 25000);   //32bytes per package, and the time interval is 25ms
                else
                {

                	if (MATLAB_TEST)
                		socketWriteNoEnd(socket_client_interface.socket_fd, (char *)(*(channel_info+i)).data, (PACKAGE_LENGTH<<1));
                	else
                		socketWriteNoEnd(fd, out, strlen(out)+1);   //+1 means sent '/0' together
                }*/

                free(out);	/* Print to text, Delete the cJSON, print it, release the string. */
                free(disp);
                PLOG("---------[package %d of channel %d is finished, the total package is %d]-------\n", j,i,data_packages);
            }

        }


        // //------------------------------------------------------------采集剩下的
        // data_residual_length = length % PACKAGE_LENGTH;
        // if (data_residual_length != 0)
        // {
        // 	PLOG("---------[data_residual_length is %d]-------\n", data_residual_length);
        // 	// set_acqusition_para(freq, data_residual_length, channel_nums, channel_list);
        // 	// channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, data_residual_length);		// free(channel_info);
        // 	acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
        // 	for (i = 0; i < channel_nums; ++i)
        // 	{
        // 		HexToStr(data_hex2str, (char *)(*(channel_info+i)).data, (data_residual_length<<1));

        // 		channel_json=cJSON_CreateObject();
        // 		cJSON_AddNumberToObject(channel_json,"num",		(*(channel_info+i)).num);
        // 		cJSON_AddStringToObject(channel_json,"name",		"virb_sens0");
        // 		cJSON_AddStringToObject(channel_json,"data",		data_hex2str);
        // 		cJSON_ReplaceItemInObject(json, "channel_info", 	channel_json);

        // 		out=cJSON_PrintUnformatted(json);
        // 		disp = cJSON_Print(json);
        // 		PLOG("%s\n",disp);
        // 		socketWriteNoEnd(socket_client_interface.socket_fd, out, strlen(out));
        // 		free(out);	/* Print to text, Delete the cJSON, print it, release the string. */
        // 		free(disp);
        // 		PLOG("---------[package %d is finished, the total package is %d]-------\n", j,data_packages);
        // 		// socketWriteNoEnd(socket_client_interface.socket_fd, (char *)(*(channel_info+i)).data, (data_residual_length<<1));
        // 		// socketWriteWithEnd(socket_client_interface.socket_fd, (char *)(*(channel_info+i)).data, (data_residual_length<<1), end_flag, 2);
        // 	}
        // }

    }
    free(channel_info);
    cJSON_Delete(json);
}



void doDataReport(char *receivedData)
{
    cJSON *json;
    int timeInterval;
    int freq,length;
    char *channels, channel_nums;
    char channel_list[8]= {0};
    // int i,j;
    PLOG("receivedData = %s\n",receivedData);

    //------------------获取参数----
    json=cJSON_Parse(receivedData);
    freq = cJSON_GetObjectItem(json,"freq")->valueint;
    length = cJSON_GetObjectItem(json,"sampleNum")->valueint;
    timeInterval = cJSON_GetObjectItem(json,"timeInterval")->valueint;
    channels = cJSON_GetObjectItem(json,"channelList")->valuestring;
    channel_nums = strlen(channels);
    strcpy(channel_list,channels);
    //执行
    set_acqusition_para(freq, length, channel_nums, channel_list);
    channel_info=malloc_result_buf(acqusition_para.valid_channel_nums, (acqusition_para.length<<1));		// free(channel_info);
    init_sigaction();
    init_time(timeInterval);
    cJSON_Delete(json);
}

void oxygenFlowReport(char *receivedData)
{
    uint16 oxygen=0;
    uint16 flow=0;
    uint16 temp=0;
    cJSON *root;
    char *out;
    oxygen=BUILD_UINT16(*(receivedData+3),*(receivedData+4));
    flow=BUILD_UINT16(*(receivedData+5),*(receivedData+6));
    temp=BUILD_UINT16(*(receivedData+7),*(receivedData+8));
    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "jsonType", JSON_TYPE_ZIGBEE_TO_GETWAY);
    cJSON_AddNumberToObject(root, "cmd", 100);
    cJSON_AddNumberToObject(root, "oxygen", oxygen);
    cJSON_AddNumberToObject(root, "flow", flow);
    cJSON_AddNumberToObject(root, "temp", temp);
    out=cJSON_PrintUnformatted(root);
    if (*(enable_mode+1)=='1')//if the app works in server model, then send the data back to client
    {
        socketWriteNoEnd(socket_server_interface.socket_fd, out, strlen(out)+1);   //+1 means sent '/0' together
        printf("%c\n", *(enable_mode+1));
    }

    if (*(enable_mode+2)=='1')//if the app works in client model, then send the data back to server
        socketWriteNoEnd(socket_client_interface.socket_fd, out, strlen(out)+1);   //+1 means sent '/0' together

    cJSON_Delete(root);
    free(out);
}



//----------------------------------------------
void sigalrm_read_ad_value(int sig)
{
    int i;
    // PLOG("alarm!\n");
    // alarm(2);
    struct timeval tpstart,tpend;
    float timeuse;

    gettimeofday(&tpstart,NULL);//开始时间


    acqusition_ad_data(ad7606_app.dev_fd, acqusition_para, channel_info);
    for (i = 0; i < acqusition_para.valid_channel_nums; ++i)
    {
        socketWriteNoEnd(socket_client_interface.socket_fd, (char *)(*(channel_info+i)).data, acqusition_para.length);
    }

    gettimeofday(&tpend,NULL);//开始时间
    //--------------计算程序用时
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
    timeuse/=1000000;
    PLOG("Used Time of is: %f\n", timeuse);

    return;
}

//------------------------------------------------
void init_sigaction(void)
{
    struct sigaction tact;
    tact.sa_handler = sigalrm_read_ad_value;
    tact.sa_flags = 0;
    sigemptyset(&tact.sa_mask);
    sigaction(SIGALRM, &tact, NULL);
}
//------------------------------------------------
void init_time(long ms)
{
    struct itimerval value;
    value.it_value.tv_sec = ms / 1000;
    value.it_value.tv_usec = ms % 1000 * 1000;
    value.it_interval = value.it_value;
    setitimer(ITIMER_REAL, &value, NULL);
}


