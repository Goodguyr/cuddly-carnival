CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:
TARGET = client

all: $(TARGET).c 
	$(CC) $(CFLAGS) -o $(TARGET) clientMain.c client.c ../protocol.c -lpthread -lraylib

test:
	$(CC) $(CFLAGS) -o $(TARGET) application.c client.c ../protocol.c -lpthread -lraylib

clean:
	$(RM) $(TARGET)