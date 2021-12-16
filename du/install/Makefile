CC=i686-w64-mingw32-gcc
LDFLAGS=-municode
CFLAGS=-DUNICODE -D_UNICODE -municode
TARGET=install.exe

all: $(TARGET)


$(TARGET): install.o
	$(CC) -o $(TARGET) $(LDFLAGS) *.o ../du/list.o ../du/error-handling.o ../du/path.o ../du/string-utils.o


clean:
	$(RM) *.o $(TARGET)
