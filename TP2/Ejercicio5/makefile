all: Servidor Cliente

Servidor: Servidor.o
	g++ -o Servidor Servidor.o -l pthread

Cliente: Cliente.o
	g++ -o Cliente Cliente.o

Servidor.o: Servidor.cpp

Cliente.o: Cliente.cpp

clean:
	rm -f Cliente Servidor *.o