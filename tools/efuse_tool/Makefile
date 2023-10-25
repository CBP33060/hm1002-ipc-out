CC = mips-linux-gnu-gcc

efuse_tool: main.o efuse.o
	$(CC) -muclibc -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -muclibc -c -o $@ $<

.PHONY: clean
clean:
	rm *.o efuse_tool 
clear:
	rm *.o 
