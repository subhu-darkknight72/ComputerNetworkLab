server : mysocket.a
	gcc -o server main_server.c mysocket.a -lpthread

client : mysocket.a
	gcc -o client main_client.c mysocket.a -lpthread 

mysocket.a : mysocket.o
	ar rcs mysocket.a mysocket.o

mysocket.o : mysocket.c
	gcc -c mysocket.c

clean :
	rm *.o *.a server client