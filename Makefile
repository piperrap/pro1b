CC=gcc
CFlags=-I.

all: 1b.o
	$(CC) -o 1b 1b.o -I.

1b: 1b.o
	$(CC) -o 1b 1b.o -I.

clean:
	rm *.o 1b
