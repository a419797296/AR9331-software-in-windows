.PHONY:clean
OBJECTS= main.o socket_driver.o socket_client.o socket_bussiness.o cJSON.o socket_server.o com_tools.o ser2net.o ad7606_app.o

spot_inspection_app:$(OBJECTS)
	mips-openwrt-linux-gcc -Wall -g $^ -o $@ -lm
main.o:main.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
socket_driver.o:socket_driver.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
socket_client.o:socket_client.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
socket_bussiness.o:socket_bussiness.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
cJSON.o:cJSON.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@ -lm
socket_server.o:socket_server.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
com_tools.o:com_tools.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
ser2net.o:ser2net.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
ad7606_app.o:ad7606_app.c
	mips-openwrt-linux-gcc -Wall -g -c $< -o $@
clean:
	rm -f $(OBJECTS) spot_inspection_app
