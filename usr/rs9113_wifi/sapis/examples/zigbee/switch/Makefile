# Makefile

rm=/bin/rm -f
CC= cc
zb= zb
DEFS=  
PROGNAME= rsi_wc_app
INCLUDES=  -I../include
LINKS= -lpthread -lrt -L../../../build/ -lrps

API_PATH = ../../../
RSI_DRIVER_PATH = ../../../include
ZB_APPLICATION_PATH = ./

DEFINES= $(INCLUDES) $(DEFS) 
CFLAGS= -g -Wall $(DEFINES) -Os
CFLAGS+= -D RSI_UART_INTERFACE
#debug prints are valid in uart case only
CFLAGS+= -D RSI_ENABLE_DEBUG_PRINT
CFLAGS+= -D RSI_SAMPLE_HAL
CFLAGS+= -D ZB_DEBUG
CFLAGS+= -D LINUX_PLATFORM
CFLAGS+= -D RSI_ZB_ENABLE
CFLAGS+=  -I$(ZB_APPLICATION_PATH)/
CFLAGS+=  -I$(RSI_DRIVER_PATH)/

ZB_SWITCH_APP = rsi_zb_app.o \
								rsi_zb_config.o

all: $(ZB_SWITCH_APP) 

	make zb CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  

clean:
	$(rm) $(ZB_SWITCH_APP) core *.o *~
