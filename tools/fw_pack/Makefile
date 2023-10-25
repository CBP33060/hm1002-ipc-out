CC = gcc
DIR = .
SRC = $(wildcard $(DIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
BIN = ipc_ota_fw_pack

CFLAG =
LDFLAG =

all: $(BIN)

$(BIN):$(OBJ)
	$(CC) -o $@ $^


.PHONY:clean all

clean:
	rm $(OBJ) $(BIN)
