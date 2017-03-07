#ifndef GPIO_H_
#define GPIO_H_


extern int numList[5];

void gpioOutHigh(int num);
void gpioOutLow(int num);
void gpioSetDirection(int num,char *direction);
void gpioSetValue(int num,char ch);
int gpioGetValue(int num);
void gpioSetValueF(FILE *fp,char ch);
int gpioGetValueF(FILE *fp);
void getValuePath(int num,char *gpio_value_path);
void getDirectionPath(int num,char *gpio_value_path);
void creatGpioFileNode(int num);
FILE * gpioInit(int num,char * direction);
void gpioRelease(int *numList);
#define Array_Len(array) (sizeof(array)/sizeof(array[0]))


#endif