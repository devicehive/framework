# use CROSS_COMPILE variable to set toolchain (empty by default):
#  >make CROSS_COMPILE=arm-linux-gnueabi-

# build variant: 'debug' or 'release' (by default)
# to change use VARIANT variable:
#  >make VARIANT=debug
#  >make VARIANT=release
variant:=release
ifdef VARIANT
  ifeq '${VARIANT}' 'release'
    variant:=release
  else ifeq '${VARIANT}' 'debug'
    variant:=debug
  else
    $(error '${VARIANT}' is unknown variant, expected: release or debug)
  endif
endif

# platform helper
ifdef PLATFORM
  platform=${PLATFORM}
else
  platform:=$(shell uname -m)
endif


ifeq '${variant}' 'debug'
  defines+=-D_DEBUG
  defines+=-g
else # default
  defines+=-DNDEBUG
  defines+=-Os
endif


CXXFLAGS+=-Wall ${include_dirs} ${defines} -std=gnu++0x
#CXXFLAGS+=-fdata-sections -ffunction-sections
#LDFLAGS+=-Wl,--gc-sections -pthread -L${ex_libs}


server: main.cpp exec.hpp gpiopin.hpp server.hpp wireless.hpp
	g++ -o server main.cpp ${CXXFLAGS} ${LDFLAGS} \
		 -liw -lmongoose -lmongocpp


#########################################################
# clean all the object files and applications
clean:
	@rm -rf *.o
	@rm -f server


.PHONY: clean 
