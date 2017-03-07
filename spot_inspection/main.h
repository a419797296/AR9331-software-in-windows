#ifndef MAIN_H_
#define MAIN_H_
#include "socket_driver.h"

/* ------------------------------------------------------------------------------------------------
 *                                               Types
 * ------------------------------------------------------------------------------------------------
 */
typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   long   int32;
typedef unsigned long   uint32;

typedef unsigned char   bool;

#define BUILD_UINT16(hiByte, loByte) \
          ((uint16)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define BUILD_UINT8(hiByte, loByte) \
          ((uint8)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))

#define HI_UINT8(a) (((a) >> 4) & 0x0F)
#define LO_UINT8(a) ((a) & 0x0F)


 #define PDEBUG				1//Byte
#ifdef PDEBUG
 #define PLOG(fmt, args...) printf(fmt, ## args)
 #else
 #define PLOG(fmt, args...)
 #endif
typedef enum
{
	JSON_TYPE_GETWAY_TO_ZIGBEE = 1,
	JSON_TYPE_ZIGBEE_TO_GETWAY, 
	JSON_TYPE_CONTROL_CMD,
	JSON_TYPE_DATA_REPOART,
	JSON_TYPE_PRODUCT_INFO,
	JSON_TYPE_OXYGEN,
	JSON_TYPE_ERROR
}jsonType;

#define HARDWARE_VERSION				"V3.0"
#define	SOFTWARE_VERSION				"20161218"

#define	LOW_POWER_ALARM_LEVEL		20

extern int DEBUG, MATLAB_TEST;
extern int package_freq;
extern char enable_mode[3];
extern char connect_flag[3];
int parse_opt(const char opt, const char *optarg);
int open_fifos(char * fifo_path, SOCKET_INTERFACE *socket_interface);
extern uint8 calc_system_soc(void);

#endif