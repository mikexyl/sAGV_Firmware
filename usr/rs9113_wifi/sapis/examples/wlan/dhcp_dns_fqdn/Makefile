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

DHCP_DNS_FQDN_APP = rsi_dhcp_dns_fqdn.o 

all: $(DHCP_DNS_FQDN_APP) 

	make wlan CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(DHCP_DNS_FQDN_APP) core *.o *~ rsi_wc_app 
