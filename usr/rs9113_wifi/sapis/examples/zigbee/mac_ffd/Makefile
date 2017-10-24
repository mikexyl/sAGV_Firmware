# Makefile

rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
INCLUDES=  -I../include
LINKS= -lpthread -lrt -L../../../build/ -lrps

API_PATH = ../../../
RSI_DRIVER_PATH = ../../../include
ZB_APPLICATION_PATH = ./
ZB_REF_APPS_SRC=$(API_PATH)/zigbee/
ZB_MAC_PATH = $(ZB_REF_APPS_SRC)/mac_apis

DEFINES= $(INCLUDES) $(DEFS) 
CFLAGS= -g -Wall $(DEFINES) -Os
#debug prints are valid in uart case only
CFLAGS+= -D RSI_ENABLE_DEBUG_PRINT
CFLAGS+= -D RSI_SAMPLE_HAL
CFLAGS+= -D RSI_ZB_ENABLE
CFLAGS+= -D ZB_MAC_API
CFLAGS+=  -I$(ZB_APPLICATION_PATH)/
CFLAGS+=  -I$(RSI_DRIVER_PATH)/
CFLAGS+=  -I$(ZB_MAC_PATH)/include

ZB_MAC_API = rsi_mac_ffd.o 

all: $(ZB_MAC_API) 

	make zb_mac CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  

clean:
	make clean CC=$(CC) -C ../../../build/ 
	$(rm) $(ZB_MAC_API) $(PROGNAME) core *.o *~
