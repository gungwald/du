CC=i686-w64-mingw32-gcc
LDFLAGS=-municode
CFLAGS=-DUNICODE -D_UNICODE -municode
TARGET=du.exe

all: $(TARGET)


$(TARGET): du.o string-utils.o error-handling.o path.o list.o du.h string-utils.h error-handling.h version.h resource.h path.h trace.h list.h
	$(CC) -o $(TARGET) $(LDFLAGS) *.o


clean:
	$(RM) *.o $(TARGET)
