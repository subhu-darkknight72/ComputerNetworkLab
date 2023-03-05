main : mysocket.a
	gcc -o main main.c mysocket.a

mysocket.a : mysocket.o
	ar rcs mysocket.a mysocket.o

mysocket.o : mysocket.c
	gcc -c mysocket.c

clean :
	rm *.o *.a
