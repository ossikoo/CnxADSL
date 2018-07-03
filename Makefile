#
# Overall makefile for the Conexant ADSL driver
#
#
DIRS=KernelModule DownLoadApp
MODULE_DIRS=KernelModule/DpController
STOP_DIRS=DownLoadApp
RPM_SOURCE_DIR=/usr/src/redhat/SOURCES
INITSCRIPTDIR=/etc/rc.d/init.d

all:
	for n in $(MODULE_DIRS); \
		do if [ -d $$n ]; then $(MAKE) -C $$n all || exit; fi; done
	for n in $(DIRS); do $(MAKE) -C $$n all || exit; done

modules:
	for n in $(MODULE_DIRS); do $(MAKE) -C $$n modules || exit; done
	for n in $(MODULE_DIRS); \
		do if [ -d $$n ]; then rm -rf $$n || exit; fi; done

clean:
	for n in $(MODULE_DIRS); \
		do if [ -d $$n ]; then $(MAKE) -C $$n clean || exit; fi; done
	for n in $(DIRS); do $(MAKE) -C $$n clean || exit; done
	make -C DownLoadApp clean

install:
	chmod a+x cnxadslctl
	cp cnxadslctl $(INITSCRIPTDIR)/
	for n in $(DIRS); do $(MAKE) -C $$n install || exit; done

uninstall:
	rm -f $(INITSCRIPTDIR)/cnxadslctl
	for n in $(DIRS); do $(MAKE) -C $$n uninstall || exit; done

start:
	for n in $(DIRS); do $(MAKE) -C $$n start || exit; done

stop:
	for n in $(STOP_DIRS); do $(MAKE) -C $$n stop || exit; done

delivery: clean
	$(error ERROR- 'delivery' no longer functional.)
	echo "*** NOT UP TO DATE ***"
	rm -rf $(RPM_SOURCE_DIR)/xyzxyz.tgz
	tar cv * | gzip > $(RPM_SOURCE_DIR)/xyzxyz.tgz
	rpm -ba xxxxxxx.spec	
	echo "*** NOT UP TO DATE ***"
