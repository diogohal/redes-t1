all:
	echo use make runs ou make runc

server: fileHandler.o rawSocketConnection.o server.o packages.o
	gcc fileHandler.o rawSocketConnection.o server.o packages.o -o server

client: fileHandler.o rawSocketConnection.o client.o packages.o
	gcc fileHandler.o rawSocketConnection.o client.o packages.o -o client

rawSocketConnection.o: rawSocketConnection.c
	gcc -c rawSocketConnection.c

client.o: client.c
	gcc -c client.c

fileHandler.o: fileHandler.c
	gcc -c fileHandler.c

packages.o: packages.c
	gcc -c packages.c

runc: client
	./client

runs: server
	./server

clean:
	rm -f *.o

purge: clean
	rm -f client