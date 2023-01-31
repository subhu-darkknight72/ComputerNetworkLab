#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int socket_desc;
    struct sockaddr_in server;
    char message[2000], server_reply[2000];
    
    // create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");
    
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    
    // connect to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected\n");
    
    // send data to server
    printf("Enter message : ");
    scanf("%s", message);
    if (send(socket_desc, message, strlen(message), 0) < 0) {
        perror("Send failed");
        return 1;
    }
    puts("Data Send\n");
    
    // receive data from server
    if (recv(socket_desc, server_reply, 2000, 0) < 0) {
        perror("recv failed");
    }
    puts("Reply received\n");
    puts(server_reply);
    
    return 0;
}
