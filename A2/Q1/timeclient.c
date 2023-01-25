// A Simple Client Implementation
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <poll.h>
#include <time.h>
#define MAXLINE 1024 
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    inet_aton("127.0.0.1", &servaddr.sin_addr); 
    
    struct pollfd S[1];
    S[0].fd = sockfd;
    S[0].events = POLLIN;

    int timeout = 3000;
    int cnt = 5;
    
    int n;
    socklen_t len; 

    //  iterate while getting timeouts
    while(cnt--){
        int p_val = poll(S,1,timeout);
        char *hello = "...CLIENT connected..."; 
        sendto(sockfd, (const char *)hello, strlen(hello), 0, 
	    		(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        // printf("Hello message sent from client\n"); 

        if(p_val<0){
            perror("Poll-Error Message !!\n");
            exit(0);
        }
        else if(p_val>0){
            
            //  successfully connected to server
            len = sizeof(servaddr);
            char buffer[MAXLINE]; 

            //  retrieve the server time
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
	        		( struct sockaddr *) &servaddr, &len); 
            buffer[n] = '\0'; 
            printf("%s\n", buffer); 
            break;
        }
    }

    if(cnt==0)
        printf("Timeout exceeded...\n");

    close(sockfd); 
    return 0; 
} 