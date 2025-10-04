# Makefile for assignment 2 compilation
#

ifeq ($(CROSS_COMPILE),)
CC = gcc
else
CC = $(CROSS_COMPILE)
endif

CFLAGS = -Wall -Werror -Wextra -O2
TARGET = writer
SOURCE = writer.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) *.o
