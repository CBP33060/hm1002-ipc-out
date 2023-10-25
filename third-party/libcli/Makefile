TARGET  := libCLILib.so

CC		:= $(CROSS_PREFIX)g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include
CFLAGS  :=  -Wall -O3 -fPIC $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -std=c++1y
SHARE   :=  -shared -o

SOURCE  :=  $(wildcard ./src/*.cpp)
OBJS    :=  $(patsubst %.cpp,%.o,$(SOURCE))

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAGS += -muclibc
endif

.PHONY : all clean install-strip
  
all : $(TARGET)

$(TARGET) : $(OBJS)
		$(CC) $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)
%.o : %.cpp
		$(CC) $(CXXFLAGS)  -c -o $@  $< $(LDFLAGS) $(LIBS)

install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m 0755 $(TARGET) $(DESTDIR)/lib
	 install -d $(DESTDIR)/include
	 cp ./include/cli $(DESTDIR)/include/ -rf

clean :
	rm $(TARGET) $(OBJS)
