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

#darwin needs bz2
ifeq ($(CONFIG_DARWIN),yes)
EXTRALIBS+=-lstdc++ -lz -lffi -lcurses
endif

#common obj
OBJS= accessor_dec.o accessor_comp.o
 
SRCS := $(OBJS:.o=.cpp)

LIB=gm_accessor_dec$(DYN_LIB_SUFFIX)


all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) $(EXTRALIBS)


clean: 
	rm -f $(OBJS) '$(OUT_PATH)'/$(LIB)

dep: depend

depend:
	rm -f .depend	
	$(CXX) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
