server : libsocket.a
	gcc -o server main_server.c libsocket.a -lpthread

client : libsocket.a
	gcc -o client main_client.c libsocket.a -lpthread 

libsocket.a : mysocket.o
	ar rcs libsocket.a mysocket.o

mysocket.o : mysocket.c
	gcc -c mysocket.c

clean :
	rm *.o *.a server client