all: ConexaoRawSocket.o t1.o
	gcc ConexaoRawSocket.o t1.o -o t1

ConexaoRawSocket.o: ConexaoRawSocket.c
	gcc -c ConexaoRawSocket.c

t1.o: t1.c
	gcc -c t1.c
