all: servidor cliente
	echo "OK"

servidor: servidor.o
	g++  -std=c++11 -o servidor servidor.o -lrt -pthread

servidor.o: servidor.cpp
	g++ -std=c++11 -c servidor.cpp 

cliente: cliente.o
	g++ -std=c++11 -o cliente cliente.o -lrt -pthread

cliente.o: cliente.cpp
	g++ -std=c++11 -c cliente.cpp 
clean:
	rm -f servidor servidor.o cliente cliente.o
