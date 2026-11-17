CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -pthread

TARGET = campuscompute
SOURCES = main.c jobs.c scheduler.c banker.c worker.c limits.c ipc.c logger.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -pthread -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
