#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gpio.h"


void gpioOutHigh(int num)
{
	char * direction="out";
	gpioSetDirection(num,direction);
	// printf("the direction of GPIO %d have been setted %s\n",num,direction);
	gpioSetValue(num,'1');
}

void gpioOutLow(int num)
{
	char * direction="out";
	gpioSetDirection(num,direction);
	gpioSetValue(num,'0');
}

void gpioSetDirection(int num,char * direction)
{
	char gpio_set_direction[50];
	char gpio_direction_path[50];
	getDirectionPath(num,gpio_direction_path);
	sprintf(gpio_set_direction,"echo %s > %s",direction,gpio_direction_path);
	// printf("system will execute the following cmd: %s\n",gpio_set_direction);
	system(gpio_set_direction);
}

void gpioSetValue(int num,char ch)
{
	
	char gpio_value_path[50];
	getValuePath(num,gpio_value_path);
	//char gpio_set_value[50];
	//sprintf(gpio_set_value,"echo %d > %s",value,gpio_value_path);
	//system(gpio_set_value);

//-----------------write by file mode

	FILE *fp;
	char ch_result;
	if ((fp=fopen(gpio_value_path,"w"))==NULL)
	{
		// printf("can't open the file");
	}
	else
	{
		ch_result=fputc(ch,fp);
		// printf("ch_result = %c;ch=%c\n", ch_result,ch);
		if(ch==ch_result)
		{
			// printf("write %c successfully\n", ch);
		}
		fclose(fp);
	}
}

int gpioGetValue(int num)
{
	
	char gpio_value_path[50];
	getValuePath(num,gpio_value_path);
	FILE *fp;
	int ch;
	if ((fp=fopen(gpio_value_path,"r"))==NULL)
	{
		printf("can't open the file");
		return -1;
	}
	else
	{
		ch=fgetc(fp);
		ch-='0';      //得到的是字符的int型，0'为48
		// printf("%d\n", ch);
		fclose(fp);
		return ch;
	}
}

//------------------------------file always open

void gpioSetValueF(FILE *fp,char ch)
{
	
	fputc(ch,fp);
}

int gpioGetValueF(FILE *fp)
{

	int ch;
	ch=fgetc(fp);
	ch-='0';      //得到的是字符的int型，0'为48
		// printf("%d\n", ch);
	return ch;
}
//-------------------------
void getValuePath(int num,char *gpio_value_path)
{
	// char gpio_value_path[50];
	// memset(gpio_value_path,0,strlen(gpio_value_path));
	sprintf(gpio_value_path,"/sys/class/gpio/gpio%d/value",num);
}

void getDirectionPath(int num,char *gpio_direction_path)
{
	// char gpio_direction_path[50];
	// memset(gpio_direction_path,0,strlen(gpio_direction_path));
	sprintf(gpio_direction_path,"/sys/class/gpio/gpio%d/direction",num);
}


void creatGpioFileNode(int num)
{
	FILE *gpio_file;
	char *gpio_node_path="/sys/class/gpio/export";
	char gpio_node_path_creat[50];
	char gpio_value_path[50];

	sprintf(gpio_node_path_creat,"echo %d > %s",num,gpio_node_path);
	// printf("%s\n",gpio_node_path_creat);
	getValuePath(num,gpio_value_path);
	gpio_file=fopen (gpio_value_path, "r");
	// printf("%s\n",gpio_value_path);
	if (gpio_file!=NULL)
	{
		// printf("%d number of GPIO drive files have already existed\n",num);
		;
		
	}
	else
	{
		if(!system(gpio_node_path_creat))
		{
			// printf("%d number of GPIO drive files creat successfully\n",num);
			;
		}
	}
}

FILE * gpioInit(int num,char * direction)
{
	char gpio_value_path[50];
	FILE *fp;
	char * file_mode;
	if (strcmp(direction,"out")==0)
	{
		file_mode="w";
	}
	else if (strcmp(direction,"in")==0)
	{
		file_mode="r";
	}
	creatGpioFileNode(num);
	gpioSetDirection(num,direction);
	getValuePath(num,gpio_value_path);
	if ((fp=fopen(gpio_value_path,file_mode))==NULL)
	{
		// printf("can't open the file");
		return NULL;
	}
	else
	{
		// printf("opened successful\n");
	}
	return fp;
}
void gpioRelease(int numList[])
{
	char releaseGpioCmd[50];
	int i;
	// printf("the nums that GPIO should release is: %d\n",Array_Len(numList));
	for (i = 0; i < 5; i++)
	{
		sprintf(releaseGpioCmd,"echo %d > /sys/class/gpio/unexport",numList[i]);
		//printf("%s\n", releaseGpioCmd);
		system(releaseGpioCmd);
	}
}