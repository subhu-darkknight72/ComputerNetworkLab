/**
 * Instructions:
 * Run sudo su before executing the code
 * gcc mytraceroute_19CS10050.c -o mytraceroute
 * ./mytraceroute <domain_name>
 */


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
#define SER_PORT 32164
#define CLI_PORT 52523
#define TTL_LIMIT 16
#define ATTEMPT_LIMIT 3
#define PAYLOAD_SIZE 52
#define BUFFER_SIZE 1024
#define TRAVEL_TIME 1


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
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
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
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_port = htons(SER_PORT); 
    inet_aton(ipAddresses, &servaddr.sin_addr);

    cliaddr.sin_family    = AF_INET; 
    cliaddr.sin_addr.s_addr = INADDR_ANY; 
    cliaddr.sin_port = htons(CLI_PORT); 
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(0); 
    }
    if ( bind(sockfd_icmp, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(0); 
    }  
    int flag = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag)) < 0) {
        perror("setsockopt");
        close(sockfd_icmp);
        close(sockfd); 
        exit(0);
    }
    int routeTraced = 0;
    for (int TTL = 1; TTL <= TTL_LIMIT; TTL++){
        if (routeTraced == 1) break;
        int attemptSuccessful = 0;
        for (int attempt = 1; attempt <= ATTEMPT_LIMIT; attempt++){
            if (attemptSuccessful == 1){
                break;
            }
            char message[PAYLOAD_SIZE + 1];
            for (int i = 0; i < PAYLOAD_SIZE - 1; i++){
                message[i] = rand() % 26 + 'a';
            }
            message[PAYLOAD_SIZE - 1] = '\0';

            struct udphdr udpHeader;
            udpHeader.source = cliaddr.sin_port;
            udpHeader.dest = servaddr.sin_port;
            udpHeader.check = 0;
            int DataGramLength = PAYLOAD_SIZE + sizeof(udpHeader);
            udpHeader.len = htons(DataGramLength);

            struct iphdr ipHeader;
            ipHeader.saddr = cliaddr.sin_addr.s_addr;
            ipHeader.daddr = servaddr.sin_addr.s_addr;
            ipHeader.ttl = TTL;
            ipHeader.ihl = sizeof(ipHeader) / 4;
            ipHeader.version = 4U;
            ipHeader.frag_off = 0;
            ipHeader.check = 0;
            ipHeader.tos = 0;
            ipHeader.id = 0;
            ipHeader.protocol = IPPROTO_UDP;
            int HeaderLen = sizeof(ipHeader) + DataGramLength;
            ipHeader.tot_len = htons(HeaderLen);

            char buffer[BUFFER_SIZE];
            memcpy(buffer,&ipHeader, sizeof(ipHeader));
            memcpy(buffer + sizeof(ipHeader), &udpHeader, sizeof(udpHeader));
            memcpy(buffer + sizeof(ipHeader) + sizeof(udpHeader), &message, PAYLOAD_SIZE);

            int r = sendto(sockfd, buffer, HeaderLen, 0, (const struct sockaddr *)&servaddr,sizeof(servaddr));
            if (r < 0){
                printf("Error,Message not Sent. \n");
                exit(0);
            }

            struct timeval startTime;
            gettimeofday(&startTime, NULL);
            struct timeval timeCounter;
            timeCounter.tv_sec = TRAVEL_TIME;
            timeCounter.tv_usec = 0;
            while (1){
                fd_set fd;
                FD_ZERO(&fd);
                FD_SET(sockfd_icmp,&fd);
                //printf("time for TTL = %d, att = %d => %d %lf\n", TTL, attempt, (int)timeCounter.tv_sec,(double)timeCounter.tv_usec);
                if (select(sockfd_icmp + 1, &fd, 0, 0, &timeCounter) < 0){
                    printf("Error in Select Call\n");
                    exit(0);
                }
                if (FD_ISSET(sockfd_icmp, &fd)){
                    struct sockaddr_in receivedFrom;
                    char msg[BUFFER_SIZE];
                    socklen_t len_rec = sizeof(receivedFrom);
                    int r = recvfrom(sockfd_icmp,msg, BUFFER_SIZE,0, (struct sockaddr *)&receivedFrom, &len_rec);
                    int msgLength = r - sizeof(struct iphdr) - sizeof(struct icmphdr);
                    struct timeval currTime;
                    gettimeofday(&currTime, NULL);
                    if (r < 0){
                        printf("Receive call error\n");
                        exit(0);
                    }
                    struct iphdr *ipHeaderRecv = (struct iphdr *)msg;
                    struct icmphdr *icmpHeaderRecv = (struct icmphdr *)(msg + sizeof(struct iphdr));
                    char *dataRecv = (char *)(msg + sizeof(struct iphdr) + sizeof(struct icmphdr));
                    if (ipHeaderRecv -> protocol != 1){
                        timeCounter.tv_sec = 0;
                        timeCounter.tv_usec = currTime.tv_usec - startTime.tv_usec;
                        continue;
                    }
                    if (icmpHeaderRecv -> type == 11){
                        struct in_addr ip_add;
                        ip_add.s_addr = ipHeaderRecv->saddr;
                        long double timeTaken = (currTime.tv_sec - startTime.tv_sec) * (1e6) + (currTime.tv_usec - startTime.tv_usec);
                        timeTaken = timeTaken / (1e6);
                        printf("Hop_Count(%d)   %-16s   %0.8Lfs\n", TTL, inet_ntoa(ip_add), timeTaken);
                        attemptSuccessful = 1;
                        break;
                    }
                    if (icmpHeaderRecv ->type == 3){
                        if (ipHeaderRecv->saddr == servaddr.sin_addr.s_addr){
                            long double timeTaken = (currTime.tv_sec - startTime.tv_sec) * (1e6) + (currTime.tv_usec - startTime.tv_usec);
                            timeTaken = timeTaken / (1e6);
                            printf("Hop_Count(%d)   %-16s   %0.8Lfs\n", TTL, inet_ntoa(servaddr.sin_addr), timeTaken);
                            attemptSuccessful = 1;
                            routeTraced = 1;
                            break;
                        }
                        else{
                            timeCounter.tv_sec = 0;
                            timeCounter.tv_usec = currTime.tv_usec - startTime.tv_usec;
                            continue;
                        }
                    }
                    timeCounter.tv_sec = 0;
                    timeCounter.tv_usec = currTime.tv_usec - startTime.tv_usec;
                }
                else {
                    break;
                }
            }

        }
        if (attemptSuccessful == 0) {
            printf("Hop_Count(%d)\t*\t*\t*\t*\n", TTL);
        }
    }
    close(sockfd);
    close(sockfd_icmp);
    return 0;
}