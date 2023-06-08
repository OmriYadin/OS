# Makefile for the bank program
CC = g++
CFLAGS = -g -Wall -std=c++11 -Werror -pedantic-errors -DNDEBUG -pthread
CCLINK = $(CC)
OBJS = bank.o rd_wr.o account.o
TARGET = Bank
RM = rm -f

# Creating the  executable
Bank: $(OBJS)
	$(CCLINK) $(CFLAGS) *.cpp -o Bank

# Creating the object files
rd_wr.o: rd_wr.cpp rd_wr.h
	$(CC) $(CFLAGS) -c rd_wr.cpp

bank.o: bank.cpp rd_wr.h account.h
	$(CC) $(CFLAGS) -c bank.cpp

account.o: account.cpp account.h rd_wr.h
	$(CC) $(CFLAGS) -c account.cpp

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core
