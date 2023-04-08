#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <linux/ip.h> 	 
#include <linux/udp.h>  
#include <linux/icmp.h>
#include <linux/tcp.h>

# define SER_PORT 30019
# define CLI_PORT 30064
# define TTL_LIMIT 16
# define ATTEMPT_LIMIT 3
# define PAYLOAD_SIZE 52
# define BUFFER_SIZE 1024
# define TRAVEL_TIME 1

int main(int argc, char *argv[]){
    srand(0);
    if (argc != 2){
        printf("Incorrect arguments.\n Format is: mytraceroute <domain> Exitting...\n");
        exit(0);
    }
    struct hostent *lh = gethostbyname(argv[1]);
    if (lh == NULL){
        printf("No ip address available.\n Exitting...\n");
        exit(0);
    }
    struct in_addr ipAddr = *(struct in_addr*) (lh->h_addr_list[0]);
    char ipAddresses[100] = {0};
    strcpy(ipAddresses,inet_ntoa(ipAddr));
    printf("Destination IP of the domain %s: %s\n",argv[1], ipAddresses);

    int sockfd, sockfd_icmp;
    struct sockaddr_in servaddr, cliaddr;
    // Create socket file descriptor
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if ( sockfd < 0 ) {
        perror("socket creation failed");
        exit(0);
    }
    sockfd_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if ( sockfd < 0 ) {
        perror("socket creation failed");
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SER_PORT);
    servaddr.sin_addr.s_addr = ipAddr.s_addr;

    // Filling client information
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(CLI_PORT);
    cliaddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket with the client address
    if ( bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 ) {
        perror("bind failed");
        exit(0);
    }
    if ( bind(sockfd_icmp, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 ) {
        perror("bind failed");
        exit(0);
    }

    int ttl = 1;
    if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0){
        perror("setsockopt failed");
        exit(0);
    }

    int route_found = 0;
    for(int ttl=0; ttl<TTL_LIMIT; ttl++){
        if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0){
            perror("setsockopt failed");
            exit(0);
        }
        int attempt = 0;
        while(attempt < ATTEMPT_LIMIT){
            int seq = rand();
            struct timeval tv;
            gettimeofday(&tv, NULL);
            long long int time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            char payload[PAYLOAD_SIZE] = {0};
            sprintf(payload, "%d %lld", seq, time);
            if(sendto(sockfd, payload, PAYLOAD_SIZE, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
                perror("sendto failed");
                exit(0);
            }
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd_icmp, &readfds);
            struct timeval timeout;
            timeout.tv_sec = TRAVEL_TIME;
            timeout.tv_usec = 0;
            int ready = select(sockfd_icmp+1, &readfds, NULL, NULL, &timeout);
            if(ready < 0){
                perror("select failed");
                exit(0);
            }
            else if(ready == 0){
                printf("Attempt %d: * * *\n", attempt+1);
                attempt++;
            }
            else{
                char buffer[BUFFER_SIZE] = {0};
                struct sockaddr_in recvaddr;
                socklen_t len = sizeof(recvaddr);
                if(recvfrom(sockfd_icmp, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &recvaddr, &len) < 0){
                    perror("recvfrom failed");
                    exit(0);
                }
                struct iphdr *ip = (struct iphdr *) buffer;
                struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof(struct iphdr));
                   
    return 0;
}