// A Simple UDP Server that sends a HELLO message
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#include <time.h>

#define MAXLINE 1024 
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    //  Server started
    printf("\nServer Running....\n");
  
    int n; 
    socklen_t len;
    char buffer[MAXLINE]; 
 
    len = sizeof(cliaddr);
    
    //  get connection-request from client, to get the client socket-id
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
			( struct sockaddr *) &cliaddr, &len); 
    buffer[n] = '\0'; 
    printf("%s\n", buffer); 

    //  get the server system time
    time_t mytime = time(NULL);
	char * time_str = ctime(&mytime);
	time_str[strlen(time_str)-1] = '\0';

    //  return the server time to the client (to the client socket-id received previously)
	sendto(sockfd, time_str, strlen(time_str) + 1, 0, 
			( struct sockaddr *) &cliaddr, sizeof(cliaddr));

    //  server closes after data-exchange is completed
    close(sockfd);
    return 0; 
} 