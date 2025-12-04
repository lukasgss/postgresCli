CC = gcc
# CFLAGS should only contain compiler flags (warnings, debug info, include paths)
CFLAGS = -Wall -Wextra -g -I/usr/include/postgresql -I.

# LDFLAGS should contain linker flags (libraries -l, library paths -L)
# The order of libraries often matters, but -lpq and -lreadline should work here.
LDFLAGS = -lpq -lreadline

# Find all .c files in current dir and subdirs
SRCS = $(wildcard *.c) $(wildcard */*.c)
OBJS = $(SRCS:.c=.o)

TARGET = pgcli

# 1. Linking Rule: Uses CFLAGS for the compilation of main, then LDFLAGS for libraries.
# The common practice is to include $(CFLAGS) here as well,
# although using just $(LDFLAGS) is often sufficient when linking object files.
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

# 2. Compilation Rule: Only uses CFLAGS for generating object files.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
