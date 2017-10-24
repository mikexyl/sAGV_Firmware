# Make File


rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../build/ -lrps

INCLUDE_PATH = ../../utilities/certificates
RSI_DRIVER_PATH = ../../../include

CFLAGS=  -I$(RSI_DRIVER_PATH)/
CFLAGS+=  -I$(INCLUDE_PATH)/
CFLAGS+= -D RSI_SAMPLE_HAL

SSL_CLIENT_APP = rsi_ssl_client.o 

all: $(SSL_CLIENT_APP) 

	make wlan CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(SSL_CLIENT_APP) core *.o *~ rsi_wc_app
