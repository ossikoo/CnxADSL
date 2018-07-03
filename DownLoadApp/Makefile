# Comment/uncomment the following line to disable/enable debugging
 DEBUG = n

# Change it here or specify it on the "make" commandline
INCLUDEDIR = /usr/include

#ifeq ($(DEBUG),y)
  #DEBFLAGS = -O -g  # "-O" is needed to expand inlines
  DEBFLAGS = -DDBG -O -ggdb  # "-O" is needed to expand inlines
#else
  DEBFLAGS = -O
#endif

#CC = kgcc
CC = cc

CFLAGS = -DCPU=486 -march=i486 -DUTS_MACHINE='"i386"' -c -Wall $(DEBFLAGS)
CFLAGS += -I$(INCLUDEDIR) -I. -I../ -I../KernelModule

TARGET = cnxadslload
OBJS = $(TARGET).o
SRC = cnxadslload.c
SRC1 = cnxadslconfig.c
SRC2 = cnxadslstatus.c
SRC3 = cnxadslautolog.c
TARGET1 = cnxadslconfig
TARGET2 = cnxadslstatus
TARGET3 = cnxadslautolog

all: .depend $(TARGET) $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET): $(SRC:.c=.o)
	$(CC) $^ -o $@

$(TARGET1): $(SRC1:.c=.o)
	$(CC) $^ -o $@

$(TARGET2): $(SRC2:.c=.o)
	$(CC) $^ -o $@

$(TARGET3): $(SRC3:.c=.o)
	$(CC) $^ -o $@

modules:
	$(error ERROR- make modules does nothing in DownloadApp.)

clean:
	rm -f cnxadslload cnxadslconfig cnxadslstatus cnxadslautolog *.o *~ core .depend

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > $@

install: cnxadslload cnxadslconfig cnxadslstatus cnxadslautolog #CLN!!!
	mkdir -p /etc/Conexant
	cp -f *.hex /etc/Conexant
	cp -f *.conf /etc/Conexant
	cp -f cnxadslload /etc/Conexant
	cp -f cnxadslconfig /usr/bin
	cp -f cnxadslstatus /usr/bin
	cp -f cnxadslautolog /usr/bin

uninstall:
	rm -rf /etc/Conexant
	rm -rf /usr/bin/cnxadslconfig
	rm -rf /usr/bin/cnxadslstatus
	rm -rf /usr/bin/cnxadslautolog

start:
	/etc/Conexant/cnxadslload /etc/Conexant &

stop:
	/sbin/rmmod CnxADSL

ifeq (.depend,$(wildcard .depend))
include .depend
endif
