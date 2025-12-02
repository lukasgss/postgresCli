CC = gcc
CFLAGS = -Wall -Wextra -g -I/usr/include/postgresql
LDFLAGS = -lpq -lreadline

# Find all .c files in current dir and subdirs
SRCS = $(wildcard *.c) $(wildcard */*.c)
OBJS = $(SRCS:.c=.o)

TARGET = pgcli

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
