DIRS = client
DIRS += server

all:
	for dir in $(DIRS); do (make -C $$dir || exit 1) || exit 1; done
clean:
	for dir in $(DIRS); do (make clean -C $$dir || exit 1) || exit 1; done

