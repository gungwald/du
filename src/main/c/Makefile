CC=gcc
LDFLAGS=-municode
CFLAGS=-DUNICODE -D_UNICODE -municode
TARGET=du.exe
OBJS=du.o string.o error.o filename.o list.o help.o args.o
HEADERS=du.h string.h error.h filename.h list.h help.h args.h getopt.h

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS)

clean:
	$(RM) *.o $(TARGET)
