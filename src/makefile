# Copyright (c) 2014 tatata1985
# All rights reserved.

CC  = gcc
TARGET  = test.bin
SRCS    = main.c \
		./utils.c \
		./worker.c \
		./csv.c

OBJS    = $(SRCS:.cpp=.o)

INCDIR  = -I. -I../main -I/usr/include
LIBDIR  = -lpthread -L/usr/lib
LIBS    = -lpthread

$(TARGET): $(OBJS)
	$(CC) -O3 -g -Wall $(INCDIR) -o $@ $^ $(LIBDIR) $(LIBS) -D__X86_64

clean:
	rm -f test.bin gmon.out
install:
	cp ./test.bin /bin/;


