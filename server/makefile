CC=$(TARGET)gcc

.PHONY: clean
.DEFAULT_GOAL=all

OBJS := main.o dial_server.o mongoose.o quick_ssdp.o url_lib.o dial_data.o system_callbacks.o
HEADERS := $(wildcard *.h)

%.c: $(HEADERS)

%.o: %.c $(HEADERS)
#	$(CC) -Wall -Werror -g -std=gnu99 $(CFLAGS) -c $*.c -o $*.o
	$(CC) -Wall -g -fPIC -std=gnu99 $(CFLAGS) -c $*.c -o $*.o

all: dialserver
debug: CFLAGS += -DDEBUG
debug: dialserver test

nf_callbacks_lib: nf_callbacks.o
#	$(CC) -Wall -Werror -g nf_callbacks.o -o libnfCallbacks.so --shared
	$(CC) -Wall -Werror -Wl,-undefined -Wl,dynamic_lookup -g nf_callbacks.o -o libnfCallbacks.so --shared

dialserver: nf_callbacks_lib $(OBJS)
	$(CC) -Wall -Werror -Wl,-rpath,. -g $(OBJS) -ldl -lpthread -lrt -L. -lnfCallbacks -o dialserver

dialserver_with_ASAN: nf_callbacks_lib $(OBJS)
	$(CC) -Wall -Werror  -fsanitize=address -Wl,-rpath,. -g $(OBJS) -ldl -lpthread -lrt -L. -lnfCallbacks -o dialserver_with_ASAN

test:
	make -C tests
	./tests/run_tests

clean:
	rm -f *.o dialserver dialserver_with_ASAN *.so
	make -C tests clean
