CC = gcc
CFLAGS = -Wall -g
INCLUDES = oufs.h oufs_lib.h vdisk.h
LIB = oufs_lib_support.o vdisk.o
EXECUTABLES = zinspect zformat zfilez zmkdir zrmdir ztouch zcreate
all: zinspect zformat zfilez zmkdir zrmdir ztouch zcreate

zinspect: zinspect.o $(LIB) $(INCLUDES)
	$(CC) zinspect.o $(LIB) -o zinspect
zformat: zformat.o $(LIB) $(INCLUDES)
	$(CC) zformat.o $(LIB) -o zformat
zfilez: zfilez.o $(LIB) $(INCLUDES)
	$(CC) zfilez.o $(LIB) -o zfilez
zmkdir: zmkdir.o $(LIB) $(INCLUDES)
	$(CC) zmkdir.o $(LIB) -o zmkdir
zrmdir: zrmdir.o $(LIB) $(INCLUDES)
	$(CC) zrmdir.o $(LIB) -o zrmdir
ztouch: ztouch.o $(LIB) $(INCLUDES)
	$(CC) ztouch.o $(LIB) -o ztouch
zcreate: zcreate.o $(LIB) $(INCLUDES)
	$(CC) zcreate.o $(LIB) -o zcreate
clean:
	rm -f $(EXECUTABLES) *.o vdisk1
