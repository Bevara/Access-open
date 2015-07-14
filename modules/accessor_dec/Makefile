include ../../config.mak

vpath %.c $(SRC_PATH)/modules/accessor_dec

CFLAGS= $(OPTFLAGS) $(CXXFLAGS) -I"$(SRC_PATH)/include" $(llvm_cxxflags)
LDFLAGS= $(llvm_ldflags)

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
CXXFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
CXXFLAGS+=-pg
LDFLAGS+=-pg
endif

EXTRALIBS+=-L../../bin/gcc -lgpac $(llvm_libs)

#common obj
OBJS_C= accessor_dec.o
OBJS_CPP= accessor_comp.o

SRCS_C := $(OBJS_C:.o=.c) 
SRCS_CPP := $(OBJS_CPP:.o=.cpp)

LIB=gm_accessor_dec$(DYN_LIB_SUFFIX)


all: $(LIB)


$(LIB): $(OBJS_C) $(OBJS_CPP)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS_C) $(OBJS_CPP) $(EXTRALIBS)


clean: 
	rm -f $(OBJS_C) $(OBJS_CPP) '$(OUT_PATH)'/$(LIB)

dep: depend

depend:
	rm -f .depend	
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
