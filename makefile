all: exoquant

exoquant.o: exoquant.c exoquant.h
	gcc -c -O2 -o exoquant.o exoquant.c

main.o: main.c exoquant.h
	gcc -c -O2 -o main.o main.c

exoquant: main.o exoquant.o
	gcc -o exoquant main.o exoquant.o -lpng -lm
