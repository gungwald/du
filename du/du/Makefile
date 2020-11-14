CC=i686-w64-mingw32-gcc
LDFLAGS=-municode
CFLAGS=-DUNICODE -D_UNICODE -municode
TARGET=du.exe

$(TARGET): du.o string-utils.o error-handling.o du.h string-utils.h error-handling.h version.h resource.h
	$(CC) -o $(TARGET) $(LDFLAGS) *.o


clean:
	$(RM) *.o $(TARGET)
