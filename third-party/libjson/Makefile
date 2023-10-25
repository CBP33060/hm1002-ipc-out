TARGET  := libjson.so

CC		:= $(CROSS_PREFIX)g++
LIBS    :=
LDFLAGS =
DEFINES :=
INCLUDE := -I./include
CFLAGS   =  -Wall -O3 -fPIC $(DEFINES) $(INCLUDE)
CXXFLAGS = $(CFLAGS)
SHARE   := -shared -o

SOURCE  :=  $(wildcard ./src/*.c)
OBJS    :=  $(patsubst %.c,%.o,$(SOURCE))

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAGS += -muclibc
endif

.PHONY : all clean install
  
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -v $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)

install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m 0755 $(TARGET) $(DESTDIR)/lib
	 install -d $(DESTDIR)/include
	 install -p -D -m 0755 ./include/* $(DESTDIR)/include

clean :
	rm $(TARGET) $(OBJS)
	    

