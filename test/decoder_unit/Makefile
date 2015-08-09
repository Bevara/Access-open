include ../../config.mak

vpath %.c $(SRC_PATH)/test/decoder_unit

CFLAGS= $(OPTFLAGS) -I"$(SRC_PATH)/include" -I../../

LINKLIBS=$(GPAC_SH_FLAGS)

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
LDFLAGS+=-pg
endif

ifeq ($(GPACREADONLY), yes)
CFLAGS+=-DGPAC_READ_ONLY
endif

#common obj
OBJS_C= terminal.o extract.o
OBJS_CPP= main.o



ifeq ($(CONFIG_WIN32),yes)
EXE=.exe
PROG=decoder_unit$(EXE)
else
EXT=
PROG=decoder_unit
endif

ifeq ($(CONFIG_DARWIN),yes)
OBJS_C+= carbon_events.o
LDFLAGS += -framework Carbon
endif

SRCS := $(OBJS_C:.o=.c) $(OBJS_CPP:.o=.cpp) 

all: $(PROG)

decoder_unit$(EXE): $(OBJS_C) $(OBJS_CPP)
	$(CXX) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS_C) $(OBJS_CPP) -L../../bin/gcc -lgpac -lgtest_main -lgtest -lpthread $(LINKLIBS)

clean: 
	rm -f $(OBJS_C) $(OBJS_CPP) ../../bin/gcc/$(PROG)

install: clean
	install -m 755 $(INSTFLAGS) ../../bin/gcc/decoder_unit "$(DESTDIR)$(prefix)/bin"

uninstall:
	rm -rf $(DESTDIR)$(prefix)/bin/decoder_unit

dep: depend

depend:
	rm -f .depend	
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend

-include .depend
