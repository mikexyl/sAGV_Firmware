# Make File

rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../build/ -lrps


RSI_DRIVER_PATH = ../../../include
RSI_APP_PATH = ../../../nwk/applications


CFLAGS=  -I$(RSI_DRIVER_PATH)/
CFLAGS+= -I$(RSI_APP_PATH)/
CFLAGS+= -D RSI_SAMPLE_HAL

MULTICAST_APP = rsi_multicast_app.o 

all:  $(MULTICAST_APP) 

	make wlan CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o 


clean:
	$(rm) $(MULTICAST_APP) $(PROGNAME) core *.o *~
