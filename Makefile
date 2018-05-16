CC=gcc
CFLAGS=-ansi -pedantic-errors -Wall

default: all	
	$(CC) -o listen main.o

all:		main.o

main.o:	main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean
clean:
	rm -rf listen main.o