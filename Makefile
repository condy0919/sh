.PHONY: all,clean

CC=clang
CFLAGS=-lreadline

all: readline.o shell.o 
	$(CC) readline.o shell.o -o sh $(CFLAGS) -g

readline.o: readline.c readline.h
	$(CC) readline.c -c -g

shell.o: shell.c shell.h
	$(CC) shell.c -c -g

clean:
	@rm *.o
