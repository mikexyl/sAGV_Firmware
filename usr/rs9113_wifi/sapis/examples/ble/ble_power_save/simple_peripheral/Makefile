# Make File


rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../../build/ -lrps


RSI_DRIVER_PATH = ../../../../include
CFLAGS+=  -I$(RSI_DRIVER_PATH)/
CFLAGS+= -D RSI_SAMPLE_HAL
CFLAGS+= -D RSI_BLE_ENABLE

BLE_PERIPHERAL_APP = rsi_ble_peripheral.o 

all: clean $(BLE_PERIPHERAL_APP) 

	make ble CC=$(CC) -C ../../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(BLE_PERIPHERAL_APP) core *.o *~ rsi_wc_app
