include ../../config.mak

vpath %.c $(SRC_PATH)/test/accessor_unit

CXXFLAGS= $(OPTFLAGS) -I"$(SRC_PATH)/include" -I../../
CFLAGS= $(OPTFLAGS) -I"$(SRC_PATH)/include" -I../../

LINKLIBS=$(GPAC_SH_FLAGS)

ifeq ($(DEBUGBUILD), yes)
CXXFLAGS+=-g
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CXXFLAGS+=-pg
CFLAGS+=-pg
LDFLAGS+=-pg
endif

ifeq ($(GPACREADONLY), yes)
CXXFLAGS+=-DGPAC_READ_ONLY
endif

#common obj
OBJS= main.o 



ifeq ($(CONFIG_WIN32),yes)
EXE=.exe
PROG=accessor_unit$(EXE)
else
EXT=
PROG=accessor_unit
endif

ifeq ($(CONFIG_DARWIN),yes)
OBJS_C+= carbon_events.o
LDFLAGS += -framework Carbon
endif

SRCS := $(OBJS:.o=.cpp) 

all: $(PROG)

accessor_unit$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) -L../../bin/gcc -lgpac -lgtest_main -lgtest -lpthread $(LINKLIBS)

clean: 
	rm -f $(OBJS) ../../bin/gcc/$(PROG)

install: clean
	install -m 755 $(INSTFLAGS) ../../bin/gcc/decoder_unit "$(DESTDIR)$(prefix)/bin"

uninstall:
	rm -rf $(DESTDIR)$(prefix)/bin/decoder_unit

dep: depend

depend:
	rm -f .depend	
	$(CC) -MM $(CXXFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend

-include .depend
