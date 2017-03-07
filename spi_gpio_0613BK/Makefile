.PHONY:clean
OBJECTS= main.o gpio.o socket_server.o cJSON.o ad7606.o ar9331_spi.o

spot_inspection_main:$(OBJECTS)
	mips-openwrt-linux-gcc -Wall -g $^ -o $@ -lm
main.o:main.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
gpio.o:gpio.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
socket_server.o:socket_server.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
ad7606.o:ad7606.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@ 
ar9331_spi.o:ar9331_spi.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@ 
cJSON.o:cJSON.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@ -lm

clean:
	rm -f $(OBJECTS) spot_inspection_main
