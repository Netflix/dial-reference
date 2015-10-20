CC=$(TARGET)gcc

.PHONY: clean
.DEFAULT_GOAL=test

OBJS := test_dial_data.o test_url_lib.o ../url_lib.o ../dial_data.o run_tests.o
HEADERS := $(wildcard ../*.h)

%.c: $(HEADERS)

%.o: %.c $(HEADERS)
	$(CC) -Wall -Werror -g -std=gnu99 $(CFLAGS) -c $*.c -o $*.o

test: $(OBJS)
	$(CC) -Wall -Werror -g $(OBJS) -ldl -lpthread -o run_tests

clean:
	rm -f *.o run_tests
