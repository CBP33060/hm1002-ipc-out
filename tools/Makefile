CC = mips-linux-gnu-gcc

PWD		:= ${ZRT_ENV_DRIVERS_DIR}/wifi/atbm_hare_iot_sdio_svn2250_20221129/atbm_hare_iot_sdio_svn
CFLAGS	:= -I ${PWD}/tools -I ${PWD}/hal_apollo

flash_od_model: flash_od_model.o
	$(CC) -muclibc -o $@ $^
atbm_iot_cli_status: atbm_iot_cli_status.o
	$(CC) -muclibc -o $@ $^
test: test.o
	$(CC) -muclibc -o $@ $^
flash_set_bad: flash_set_bad.o
	$(CC) -muclibc -o $@ $^
%.o:%.c
	$(CC) $(CFLAGS) -muclibc -c -o $@ $<

.PHONY: clean
clean:
	rm *.o flash_od_model atbm_iot_cli_status
clear:
	rm *.o 
