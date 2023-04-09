# Makefile for the smash program
CC = g++
CFLAGS = -g -Wall -std=c++11
CCLINK = $(CC)
OBJS = smash.o commands.o signals.o
RM = rm -f

# Creating the  executable
smash: $(OBJS)
	$(CCLINK) -o smash $(OBJS)

# Creating the object files
commands.o: commands.cpp commands.h
	$(CC) $(CCFLAGS) -c commands.cpp

smash.o: smash.cpp commands.h
	$(CC) $(CCFLAGS) -c smash.cpp

signals.o: signals.cpp signals.h
	$(CC) $(CCFLAGS) -c signals.cpp

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core