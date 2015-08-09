include ../config.mak

#all OS and lib independent
TESTDIRS=

ifneq ($(CONFIG_GTEST), no)
TESTDIRS+=decoder_unit
endif


all: tests

tests:	
	set -e; for i in $(TESTDIRS) ; do $(MAKE) -C $$i all; done 

dep:
	set -e; for i in $(TESTDIRS) ; do $(MAKE) -C $$i dep; done 

clean:
	set -e; for i in $(TESTDIRS) ; do $(MAKE) -C $$i clean; done 

distclean:	
	set -e; for i in $(TESTDIRS) ; do $(MAKE) -C $$i distclean; done 
