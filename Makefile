all: rawSocketConnection.o client.o packages.o
	gcc rawSocketConnection.o client.o packages.o -o client

rawSocketConnection.o: rawSocketConnection.c
	gcc -c rawSocketConnection.c

client.o: client.c
	gcc -c client.c

packages.o: packages.c
	gcc -c packages.c

run: all
	./client

clean:
	rm -f *.o

purge: clean
	rm -f client