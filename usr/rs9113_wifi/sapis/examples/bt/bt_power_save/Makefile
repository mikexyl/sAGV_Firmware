##----------------------------------------------------------------------------
# Created with genmake.pl v1.1 on Thu Aug 20 16:58:17 2015
# genmake.pl home: http://muquit.com/muquit/software/
# Copryright: GNU GPL (http://www.gnu.org/copyleft/gpl.html)
##----------------------------------------------------------------------------
rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= rsi_wc_app
LINKS= -lpthread -L../../../build/ -lrps


RSI_DRIVER_PATH = ../../../include
CFLAGS+=  -I$(RSI_DRIVER_PATH)/
CFLAGS+= -D RSI_SAMPLE_HAL
CFLAGS+= -D RSI_BT_ENABLE

BT_PWR_SAVE_APP = rsi_bt_power_save_profile.o 

all: clean $(BT_PWR_SAVE_APP) 

	make bt_classic CC=$(CC) -C ../../../build/ 
	$(CC) $(CFLAGS) *.o $(LINKS) -o $(PROGNAME) 
	rm -f  *.o  


clean:
	$(rm) $(BT_SPP_SLAVE_APP) core *.o *~ rsi_wc_app
