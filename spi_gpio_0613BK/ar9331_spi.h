#ifndef AR9331_SPI_H_
#define AR9331_SPI_H_

#define SPI_DEV_PATH    "/dev/spidev1.0"

#define SPI_CV_AB			22
#define SPI_CLK			18
#define SPI_MISO			20

void readAD(int freq, int sampleNum,unsigned char * ad_result);
void reportAD(char *devName, int timeInterval,unsigned char * ad_result);
// static void do_read(int fd, unsigned char * ad_result,int len);
// static void dumpstat(const char *name, int fd);

#endif