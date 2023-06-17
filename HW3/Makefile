# Makefile for the bank program
CC = g++
CFLAGS = -g -Wall -std=c++11 -Werror -pedantic-errors -DNDEBUG -pthread
CCLINK = $(CC)
OBJS = ttftps.o
TARGET = ttftps
RM = rm -f

# Creating the  executable
ttftps: $(OBJS)
	$(CCLINK) $(CFLAGS) *.cpp -o ttftps

# Creating the object files

ttftps.o: ttftps.cpp
	$(CC) $(CFLAGS) -c ttftps.cpp

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core
