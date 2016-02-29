include ../config.mak

vpath %.c $(SRC_PATH)/src
vpath %.cpp $(SRC_PATH)/src

CFLAGS= $(OPTFLAGS) $(CXXFLAGS) -I"$(SRC_PATH)/include" $(llvm_cxxflags)
LDFLAGS= $(llvm_ldflags)

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
LDFLAGS+=-pg
endif

EXTRALIBS+=-L../bin/gcc -lgpac $(llvm_libs)

LIBACCESS_UTILS_C=
LIBACCESS_UTILS_CPP=
ifeq ($(CONFIG_LLVM), system)
LIBACCESS_UTILS_C+=utils/accessor.o utils/os_accessor.o 
LIBACCESS_UTILS_CPP+= utils/compiler.o
endif


## libgpac objects gathering: 
OBJS_C=$(LIBACCESS_UTILS_C)
OBJS_CPP=$(LIBACCESS_UTILS_CPP)

##libgpac library output
LIB=libaccess$(DYN_LIB_SUFFIX)
ifeq ($(CONFIG_WIN32),yes)
LDFLAGS+=-Wl,-out-implib=../bin/gcc/libaccess.dll.a
else
ifeq ($(CONFIG_DARWIN),yes)
LDFLAGS+=-dynamiclib -install_name $(prefix)/lib/$(LIB)
EXTRALIBS+=-lz -lncurses
endif
EXTRALIBS+=$(GPAC_SH_FLAGS)
endif

LD_SONAME="-Wl,-soname,libaccess.so.$(VERSION_MAJOR)"
ifeq ($(CONFIG_DARWIN), yes)
LD_SONAME=
endif

ifeq ($(CONFIG_SUNOS), yes)
LD_SONAME="-Wl,-h,$(LIB)"
LINKLIBS+= -lrt
endif

SRCS_C := $(OBJS_C:.o=.c)
SRCS_CPP := $(OBJS_CPP:.o=.cpp)  

all: lib

lib: ../bin/gcc/$(LIB)


../bin/gcc/$(LIB): $(LIBACCESS_UTILS_C) $(LIBACCESS_UTILS_CPP)  $(OBJS_C) $(OBJS_CPP)
	@echo "OBJS_C $(OBJS_C)"
	@echo "OBJS_CPP $(OBJS_CPP)"
	@echo "LIBS $(EXTRALIBS)"

ifeq ($(CONFIG_DARWIN),yes)

	$(LIBTOOL) -s -o ../bin/gcc/libaccess_static.a $(OBJS_C) $(OBJS_CPP)
	$(RANLIB) ../bin/gcc/libaccess_static.a
ifneq ($(STATICBUILD),yes)
	$(CXX) $(SHFLAGS) $(LD_SONAME) $(LDFLAGS) -o $@ $(OBJS_C) $(OBJS_CPP) $(EXTRALIBS) 
endif

else

	$(AR) cr ../bin/gcc/libaccess_static.a $(OBJS)
	$(RANLIB) ../bin/gcc/libaccess_static.a
ifneq ($(STATICBUILD),yes)
	$(CC) $(SHFLAGS) $(LD_SONAME) $(LDFLAGS) -o $@ $(OBJS) $(EXTRALIBS)	
	mv $@ $@.$(VERSION_SONAME)
	ln -sf $(notdir $@).$(VERSION_SONAME) $@.$(VERSION_MAJOR)
	ln -sf $(notdir $@).$(VERSION_SONAME) $@
endif

endif
ifeq ($(shell fgrep "Libs.private:" ../gpac.pc),)
	@echo "Libs.private: $(EXTRALIBS)" >> ../gpac.pc
endif

dep:

DEPS := $(SRCS_C:%.c=.deps/%.dep) $(SRCS_CPP:%.cpp=.deps/%.dep)

-include .depend

.depend: $(DEPS)
	cat $^ > "$@"

.deps/%.dep: %.c
	@mkdir -p $(dir $@)
	$(CC) -MM $(CFLAGS) "$<" -MT "$*.o" -MF "$@"

clean: 
	rm -f $(OBJS) ../bin/gcc/$(LIB)
	rm -rf ../bin/gcc/libaccess_static.a ../bin/gcc/libaccess.so \
		../bin/gcc/libaccess.so.$(VERSION_MAJOR) \
		../bin/gcc/libaccess.so.$(VERSION_SONAME)

distclean: clean
	rm -f Makefile.bak .depend $(DEPS)
