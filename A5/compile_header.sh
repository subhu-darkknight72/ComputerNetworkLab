rm *.o *.a server
gcc -c mysocket.c
ar rcs mysocket.a mysocket.o
gcc -o server main_server.c mysocket.a -lpthread
./server