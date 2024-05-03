all: procesoA procesoB

procesoA: procesoA.o
	gcc -o procesoA procesoA.o

procesoB: procesoB.o
	gcc -o procesoB procesoB.o

procesoA.o: procesoA.c

procesoB.o: procesoB.c

clean:
	rm -f *.o