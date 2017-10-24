# Make File

rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../build/ -lrps

RSI_DRIVER_PATH = ../../../include

CFLAGS=  -I$(RSI_DRIVER_PATH)/
CFLAGS+=  -I$(INCLUDE_PATH)/
CFLAGS+= -DRSI_SAMPLE_HAL
CFLAGS+= -D RSI_BLE_ENABLE

WLAN_AP_BLE_BRIDGE_APP = main.o \
                      rsi_wlan_ap_app.o \
                      rsi_ble_app.o  

all: $(WLAN_AP_BLE_BRIDGE_APP) 

	make wlan_ble CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(WLAN_AP_BLE_BRIDGE_APP) core *.o *~ rsi_wc_app
