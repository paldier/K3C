CC = gcc
CFLAGS = -W -I ../include
LDFLAGS = -lgearman
TARGET = test

all:$(TARGET)

test:
	$(CC) $(CFLAGS) test_gearman_client.c ../src/ngx_http_gearman_client.c $(LDFLAGS) -o $@

clean:
	rm test
