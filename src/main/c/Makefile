CC=x86_64-w64-mingw32-gcc
CC=i686-w64-mingw32-gcc
LDFLAGS=-municode
# Using -l:libgc.a causes libwinpthreads to get dynamically linked, but this does not:
LDLIBS=-Wl,-Bstatic -lgc
CFLAGS=-DUNICODE -D_UNICODE -municode -Wall -g
TARGET=du.exe
INSTALLER=du-setup.exe
OBJS=du.o string.o error.o filename.o list.o help.o args.o registry.o
HEADERS=du.h string.h error.h filename.h list.h help.h args.h getopt.h registry.h

all: build-number $(TARGET) $(INSTALLER)

$(INSTALLER): $(TARGET)
	cp $(TARGET) $(INSTALLER)

build-number:
	..\..\build\c\increment-build-number.exe build-number.h

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS) $(TARGET)
