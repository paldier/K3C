##Make file for reachdip

CFLAGS += -O2 $(LOGGER_CFG_FLAG) -I./

all: readchip

readchip: readchip.o mem.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	/bin/rm -f *.o readchip
