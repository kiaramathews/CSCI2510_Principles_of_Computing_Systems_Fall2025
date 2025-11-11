CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = slush

all: $(TARGET)

$(TARGET): slush.c
	$(CC) $(CFLAGS) -o $(TARGET) slush.c

clean:
	rm -f $(TARGET)

.PHONY: all clean