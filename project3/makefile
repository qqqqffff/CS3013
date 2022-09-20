CC := gcc
CFLAGS := -g -Wall

all: memorymanager

memorymanager: MemoryManager.c
	$(CC) $(CFLAGS) -o memorymanager MemoryManager.c -lm

clean:
	$(RM) memorymanager