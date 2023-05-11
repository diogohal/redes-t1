all: rawSocketConnection.o client.o
	gcc rawSocketConnection.o client.o -o client

rawSocketConnection.o: rawSocketConnection.c
	gcc -c rawSocketConnection.c

client.o: client.c
	gcc -c client.c

run: all
	./client

clean:
	rm -f *.o

purge: clean
	rm -f client