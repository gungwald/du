CC=i686-w64-mingw32-gcc
CFLAGS=-Wall -g
TARGET=increment-build-number.exe
SRCS=increment-build-number.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $@ $(CFLAGS) $(SRCS)
	
clean:
	$(RM) $(TARGET)
