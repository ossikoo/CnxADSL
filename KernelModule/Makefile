ifeq ($(KDIR),)
	KDIR := /usr/src/linux
endif
EXTRA_CFLAGS := -DNDIS_MINIPORT_DRIVER -DOS_LINUX -Wno-multichar -fno-builtin
#EXTRA_LDFLAGS := -r

all:
	make CnxADSL.ko

obj-m := CnxADSL.o
CnxADSL-objs := ARMAbstract.o BufMgmt.o CardAL.o CardALADSLDiag.o CardALBdDp.o CardALBd.o CardALTigris.o CardALTigrisDiag.o CardALTigrisLC.o CardMgmt.o \
		CardMgmtLink.o CardMgmtVc.o CellDataTest.o CellDataTestMgmt.o ChipALBusCtlP46.o ChipALCdsl.o ChipALDMAChanP46.o ChipALIoP46.o CardALTigrisDp.o \
		CardALTigrisHal.o ChipALSEmw.o CnxTTY.o FrameAl.o FrameALAAL.o FrameALATM.o FrameALATMOAM.o FrameALATMShaper.o FrameALHec.o KThread.o \
		LnxTools.o SmLnxIf.o SmSysIf.o UtilStr.o UtilTmr.o dpcontroller.o

dpcontroller.o: dpcontroller.o.new
	objcopy -L memcpy -L memset -L memmove -L memcmp --redefine-sym vsprintf=vsprintf_dpc $< $@

CnxADSL.ko: dpcontroller.o
	make -C ${KDIR} SUBDIRS=`pwd` modules

install: CnxADSL.ko
	make -C ${KDIR} SUBDIRS=`pwd` modules_install

clean:
	rm -f *.o *~ core .depend dep *.ko CnxADSL.mod.c .*.cmd dpcontroller.o
	rm -rf .tmp_versions/
