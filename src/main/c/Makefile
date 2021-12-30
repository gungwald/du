CC=x86_64-w64-mingw32-gcc
CC=i686-w64-mingw32-gcc
LDFLAGS=-municode
# Using -l:libgc.a causes libwinpthreads to get dynamically linked, but this does not:
LDLIBS=-Wl,-Bstatic -lgc
CFLAGS=-DUNICODE -D_UNICODE -municode -Wall
TARGET=du.exe
INSTALLER=du-setup.exe

.PHONY: all clean debug release remake

all: debug

# Use the shell find command to get the source tree
SRCS := $(shell find * -type f -name "*.c")
OBJS = $(SRCS:.c=.o)

DEBUG_DIR=Debug
DEBUG_TARGET=$(DEBUG_DIR)/$(TARGET)
DEBUG_OBJS=$(addprefix $(DEBUG_DIR)/, $(OBJS))
DEBUG_FLAGS=-g
DEBUG_INSTALLER=$(DEBUG_DIR)/$(INSTALLER)

debug: build-number $(DEBUG_DIR) $(DEBUG_INSTALLER)

$(DEBUG_INSTALLER): $(DEBUG_TARGET)
	cp $(DEBUG_TARGET) $(DEBUG_INSTALLER)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(CC) -o $(DEBUG_TARGET) $(LDFLAGS) $^ $(LDLIBS)

$(DEBUG_OBJS): $(DEBUG_DIR)/%.o: %.c
	$(CC) -MMD -MP $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

-include $(DEBUG_OBJS:.o=.d)


RELEASE_DIR = Release
RELEASE_TARGET = $(RELEASE_DIR)/$(TARGET)
RELEASE_OBJS = $(addprefix $(RELEASE_DIR)/, $(OBJS))
RELEASE_FLAGS = -O3
RELEASE_INSTALLER=$(RELEASE_DIR)/$(INSTALLER)

release: build-number $(RELEASE_DIR) $(RELEASE_INSTALLER)

$(RELEASE_INSTALLER): $(RELEASE_TARGET)
	cp $(RELEASE_TARGET) $(RELEASE_INSTALLER)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) -o $(RELEASE_TARGET) $(LDFLAGS) $^ $(LDLIBS)
	strip $(RELEASE_TARGET)

$(RELEASE_OBJS): $(RELEASE_DIR)/%.o: %.c
	$(CC) -MMD -MP $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

-include $(RELEASE_OBJS:.o=.d)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)


build-number:
	..\..\build\c\increment-build-number.exe build-number.h

remake: clean all

clean:
	$(RM) -r $(RELEASE_DIR) $(DEBUG_DIR)
