# Makefile for the bank program
CC = g++
CFLAGS = -g -Wall -std=c++11 -Werror -pedantic-errors -DNDEBUG
CCLINK = $(CC)
OBJS = bank.o rd_wr.o account.o
TARGET = bank
RM = rm -f

# Creating the  executable
bank: $(OBJS)
	$(CCLINK) -o bank $(OBJS)

# Creating the object files
rd_wr.o: rd_wr.cpp rd_wr.h
	$(CC) $(CCFLAGS) -c rd_wr.cpp

bank.o: bank.cpp rd_wr.h account.h
	$(CC) $(CCFLAGS) -c bank.cpp

account.o: account.cpp account.h rd_wr.h
	$(CC) $(CCFLAGS) -c account.cpp

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core
