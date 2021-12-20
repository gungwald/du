# i686-w64-mingw32-gcc is a name that is consistent across Linux and Windows.
CC=x86_64-w64-mingw32-gcc
CC=i686-w64-mingw32-gcc
LDFLAGS=-municode
# Using -l:libgc.a causes libwinpthreads to get dynamically linked, but this does not:
LDLIBS=-Wl,-Bstatic -lgc
CFLAGS=-DUNICODE -D_UNICODE -municode -Wall -g
TARGET=du.exe
OBJS=du.o string.o error.o filename.o list.o help.o args.o
HEADERS=du.h string.h error.h filename.h list.h help.h args.h getopt.h

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS) $(TARGET)
