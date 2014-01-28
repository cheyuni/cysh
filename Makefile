.SUFFIXES : .c .o
CC = gcc
OBJS = cysh.o 
SRCS = cysh.c cysh.h

TARGET = cysh

all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)

clean :
	rm -rf $(OBJS) $(TARGET) 

