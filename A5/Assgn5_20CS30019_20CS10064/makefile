server_filename=main_server.c
client_filename=main_client.c

server : libsocket.a
	gcc -o server $(server_filename) libsocket.a -lpthread

client : libsocket.a
	gcc -o client $(client_filename) libsocket.a -lpthread 

libsocket.a : mysocket.o
	ar rcs libsocket.a mysocket.o

mysocket.o : mysocket.c
	gcc -c mysocket.c

clean :
	rm *.o *.a server client