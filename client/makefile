CC=$(TARGET)g++

.PHONY: clean
.DEFAULT_GOAL=dialclient

includes = $(wildcard *.h)
OBJS := main.cpp DialServer.cpp DialDiscovery.cpp DialConformance.cpp DialClientInput.cpp

# You may not need all these libraries.  This example uses a build of curl that needs crypto, ssl, cares, and zlib
dialclient: $(OBJS) ${includes}
	$(CC) -Wall -Werror -g $(OBJS) $(INCLUDES) $(LDFLAGS) -ldl -lpthread -lcurl -lz -lcrypto -lssl -lcares -m32 -o dialclient

clean:
	rm -f *.o dialclient
