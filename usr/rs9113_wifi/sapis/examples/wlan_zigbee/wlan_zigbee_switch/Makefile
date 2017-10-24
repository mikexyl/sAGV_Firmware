# Make File

rm=/bin/rm -f
CC= cc 
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../build/ -lrps

INCLUDE_PATH = ../../utilities/certificates
RSI_DRIVER_PATH = ../../../include
API_PATH = ../../../
RSI_DRIVER_PATH = ../../../include
ZB_APPLICATION_PATH = ./

CFLAGS= -g -Wall $(DEFINES) -Os
CFLAGS+= -D RSI_ZB_ENABLE
CFLAGS+= -D LINUX_PLATFORM
CFLAGS+= -D ZB_DEBUG
CFLAGS+= -D RSI_SAMPLE_HAL
CFLAGS+=  -I$(RSI_DRIVER_PATH)/
CFLAGS+=  -I$(INCLUDE_PATH)/
CFLAGS+=  -I$(ZB_APPLICATION_PATH)/
CFLAGS+=  -I$(RSI_DRIVER_PATH)/

WLAN_ZIGBEE_BRIDGE_APP = main.o \
                      rsi_wlan_app.o \
                      rsi_zb_config.o \
                      rsi_zb_app.o  

all: $(WLAN_ZIGBEE_BRIDGE_APP) 

	make wlan_zb CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(WLAN_ZIGBEE_BRIDGE_APP) core *.o *~
