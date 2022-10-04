CC := gcc
CFLAGS := -g -Wall

all: scheduler

scheduler: scheduler.c
	$(CC) $(CFLAGS) -o scheduler scheduler.c -lm

clean:
	$(RM) memorymanager