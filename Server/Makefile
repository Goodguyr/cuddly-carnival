CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:
TARGET = server

all: $(TARGET).c game.c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c game.c ../protocol.c -lraylib

clean:
	$(RM) $(TARGET)