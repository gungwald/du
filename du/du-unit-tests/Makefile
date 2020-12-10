CC=i686-w64-mingw32-gcc
CXX=i686-w64-mingw32-g++
LDFLAGS=-municode
CFLAGS=-DUNICODE -D_UNICODE -municode -v
CXXFLAGS=-DUNICODE -D_UNICODE -municode -v -I/usr/include
TARGET=test.exe

all: $(TARGET)


$(TARGET): test.o
	$(CXX) -o $(TARGET) $(LDFLAGS) *.o


clean:
	$(RM) *.o $(TARGET)
