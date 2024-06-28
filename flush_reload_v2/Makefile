CC = gcc
CFLAGS = -g -Wall
RM = rm -f
INCLUDES = ./

all: sender receiver

sender: sender.o util.o
	$(CC) $(CFLAGS) -o sender sender.o util.o

receiver: receiver.o util.o
	$(CC) $(CFLAGS) -o receiver receiver.o util.o

sender.o: sender.c util.h sender.h constants.h
	$(CC) $(CFLAGS) -c sender.c

receiver.o: receiver.c util.h receiver.h constants.h
	$(CC) $(CFLAGS) -c receiver.c

util.o: util.c util.h constants.h
	$(CC) $(CFLAGS) -c util.c

clean:
	$(RM) *.o sender receiver
