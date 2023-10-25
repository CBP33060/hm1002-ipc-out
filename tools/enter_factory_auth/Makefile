HOST_CC = gcc
HOST_CFLAGS = -O2 -Wall -I./include
HOST_LDFLAG = -lssl -lcrypto

CROSS_CC = mips-linux-gnu-gcc
CROSS_CFLAGS = -O2 -Wall -muclibc -I./include -L./lib  -L../../lib/uclibc -I../../include -L../../libcrypt/lib/static_lib -I../../libcrypt/include
CROSS_LDFLAG = -ljson -lmbedcrypto -lmbedtls $(LDFLAGS)

PROGS = enter_factory_auth_test enter_factory_auth 

all: $(PROGS)

enter_factory_auth_test: test.host.o io.host.o
	$(HOST_CC) -o $@ $^ $(HOST_LDFLAG) 

enter_factory_auth: enter_factory_auth.cross.o io.cross.o
	$(CROSS_CC) -o $@ $^ $(CFLAGS) $(CROSS_CFLAGS) $(CROSS_LDFLAG) 

%.host.o:%.c
	$(HOST_CC) -c $(HOST_CFLAGS) $< -o $@

%.cross.o:%.c
	$(CROSS_CC) -c  $< -o $@ $(CFLAGS) $(CROSS_CFLAGS) $(CROSS_LDFLAG) 

clean:
	-rm -f *.o *~ 

distclean: clean
	-rm -f $(PROGS)