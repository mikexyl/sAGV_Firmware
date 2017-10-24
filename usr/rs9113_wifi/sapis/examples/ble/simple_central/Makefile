# Make File


rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../build/ -lrps


RSI_DRIVER_PATH = ../../../include
CFLAGS+=  -I$(RSI_DRIVER_PATH)/
CFLAGS+= -D RSI_SAMPLE_HAL
CFLAGS+= -D RSI_BLE_ENABLE

BLE_CENTRAL_APP = rsi_ble_central.o 

all: $(BLE_CENTRAL_APP) 

	make ble CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(BLE_CENTRAL_APP) core *.o *~
