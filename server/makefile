CC=$(TARGET)gcc

.PHONY: clean
.DEFAULT_GOAL=all

OBJS := main.o dial_server.o mongoose.o quick_ssdp.o url_lib.o dial_data.o
HEADERS := $(wildcard *.h)

%.c: $(HEADERS)

%.o: %.c $(HEADERS)
	$(CC) -Wall -Werror -g -std=gnu99 $(CFLAGS) -c $*.c -o $*.o

all: dialserver test

dialserver: $(OBJS)
	$(CC) -Wall -Werror -g $(OBJS) -ldl -lpthread -o dialserver

test:
	make -C tests
	./tests/run_tests

clean:
	rm -f *.o dialserver
	make -C tests clean
