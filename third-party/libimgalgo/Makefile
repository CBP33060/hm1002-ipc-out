TOPDIR ?= ./InferenceKit
CROSS_COMPILE:=mips-linux-gnu-
TARGET := libimgalgo.so

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
libtype ?= muclibc
build_type ?= release
#build_type ?= profile
#build_type ?= debug
#build_type ?= nmem

CXXFLAGS := -g -rdynamic -std=c++11 -mfp64 -mnan=2008 -mabs=2008 -Wall -EL -O3 -march=mips32r2 -flax-vector-conversions -lpthread -lrt -ldl -lm -fPIC
SHARE   := -shared -o

INCLUDES := -I$(TOPDIR)/include 
INCLUDES += -I./stb
INCLUDES += -I./include
CXXFLAGS += $(INCLUDES)

ifeq ($(libtype), muclibc)
	CXXFLAGS += -muclibc
endif

ifeq ($(build_type), release)
    ifeq ($(libtype), muclibc)
        LDFLAGS := -L$(TOPDIR)/lib/uclibc -lvenus -laip -ldrivers -lalgsdk -lmi_alg_t41_detector_sdk 
    else
        LDFLAGS := -L$(TOPDIR)/lib/glibc -lvenus -laip -ldrivers
    endif

else ifeq ($(build_type), profile)
    CXXFLAGS += -DVENUS_PROFILE
    ifeq ($(libtype), muclibc)
        LDFLAGS := -L$(TOPDIR)/lib/uclibc/ -lvenus.p -laip -ldrivers
    else
        LDFLAGS := -L$(TOPDIR)/lib/glibc/ -lvenus.p -laip -ldrivers
    endif

else ifeq ($(build_type), debug)
    CXXFLAGS += -DVENUS_DEBUG
    ifeq ($(libtype), muclibc)
        LDFLAGS := -L$(TOPDIR)/lib/uclibc/  -lvenus.d -laip -ldrivers
    else
        LDFLAGS := -L$(TOPDIR)/lib/glibc/  -lvenus.d -laip -ldrivers
    endif

else ifeq ($(build_type), nmem)
    ifeq ($(libtype), muclibc)
        LDFLAGS := -L$(TOPDIR)/lib/uclibc/ -lvenus -laip -ldrivers.m
    else
        LDFLAGS := -L$(TOPDIR)/lib/glibc/ -lvenus -laip -ldrivers.m
    endif

endif

SRC1 = $(wildcard ./src/*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRC1))

%.o:%.cpp
	$(CPP) $(INCLUDES) $(CXXFLAGS) -o $@ -c $^

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)

install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m  0755 $(TARGET) $(DESTDIR)/lib
	 install -d $(DESTDIR)/include
	 cp ./include/* $(DESTDIR)/include/ -rf
	 rm $(DESTDIR)/include/Eigen -rf
.PHONY: clean
clean: 
	rm -f $(TARGET) $(OBJS) install.log make.log
