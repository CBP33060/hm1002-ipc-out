TARGET  := librcf.so
CC      := $(CROSS_PREFIX)g++
LIBS    = -luuid
LDFLAGS =
DEFINES :=
INCLUDE = -I./include
CFLAGS   =  -Wall -O3 -fPIC $(DEFINES) $(INCLUDE)
CXXFLAGS = $(CFLAGS) -std=c++1y
SHARE   :=  -shared -o

SOURCE  := src/RCF/RCF.cpp
OBJS    := src/RCF/RCF.o

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAGS += -muclibc
LIBS    += -L../target/uclibc/libuuid/usr/local/lib
INCLUDE += -I../target/uclibc/libuuid/usr/local/include
else
LIBS    += -L../target/glibc/libuuid/usr/local/lib
INCLUDE += -I../target/glibc/libuuid/usr/local/include
endif

.PHONY : all clean install-strip
  
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)

$(OBJS) : $(SOURCE)
	$(CC) $(CXXFLAGS) -c -o $@  $< $(LDFLAGS) $(LIBS)

install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m  0755 $(TARGET) $(DESTDIR)/lib
	#  install -d $(DESTDIR)/include
	#  cp ./include/* $(DESTDIR)/include/ -rf

clean :
	rm $(TARGET) $(OBJS)
