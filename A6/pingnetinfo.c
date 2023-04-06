#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

#define PORT 8080

uint16_t checksum(const void *buff, size_t nbytes)
{
    uint64_t sum = 0;
    uint16_t *words = (uint16_t *)buff;
    size_t _16bitword = nbytes / 2;
    // Sum all the 16-bit words
    while (_16bitword--)
    {
        sum += *(words++);
    }
    // Add left-over byte, if any
    if (nbytes & 1)
    {
        sum += (uint16_t)(*(uint8_t *)words) ;
    }
    sum = ((sum >> 16) + (sum & 0xFFFF));   // Fold to 16 bits
    sum += (sum >> 16); // Add carry
    return (uint16_t)(~sum);    // Return one's complement
}
void printIP(struct iphdr *ip)
{
    printf("-----------------------------------------------------------------\n");
    printf("|   version:%-2d  |   hlen:%-4d   |     tos:%-2d    |  totlen:%-4d  |\n", ip->version, ip->ihl, ip->tos, ip->tot_len);
    printf("-----------------------------------------------------------------\n");
    printf("|           id:%-6d           |%d|%d|%d|      frag_off:%-4d      |\n", ntohs(ip->id), ip->frag_off && (1 << 15), ip->frag_off && (1 << 14), ip->frag_off && (1 << 14), ip->frag_off);
    printf("-----------------------------------------------------------------\n");
    printf("|    ttl:%-4d   |  protocol:%-2d  |         checksum:%-6d       |\n", ip->ttl, ip->protocol, ip->check);
    printf("-----------------------------------------------------------------\n");
    printf("|                    source:%-16s                    |\n", inet_ntoa(*(struct in_addr *)&ip->saddr));
    printf("-----------------------------------------------------------------\n");
    printf("|                 destination:%-16s                  |\n", inet_ntoa(*(struct in_addr *)&ip->daddr));
    printf("-----------------------------------------------------------------\n");
}
void printICMP(struct icmphdr *icmp)
{
    printf("-----------------------------------------------------------------\n");
    printf("|    type:%-2d    |    code:%-2d    |        checksum:%-6d        |\n", icmp->type, icmp->code, icmp->checksum);
    printf("-----------------------------------------------------------------\n");
    if (icmp->type == ICMP_ECHO || icmp->type == ICMP_ECHOREPLY)
        printf("|           id:%-6d           |        sequence:%-6d        |\n", icmp->un.echo.id, icmp->un.echo.sequence);
    printf("-----------------------------------------------------------------\n");
}
int main(){
    printf("Enter hostname/ip address: ");
    char host[1000];
    scanf("%s", host);

    struct hostent *h = gethostbyname(host);
    if (h == NULL){
        printf("gethostbyname failed\n");
        exit(1);
    }   
    char* ip_address = malloc(1000);
    ip_address = inet_ntoa(*((struct in_addr *)h->h_addr));
    printf("IP address: %s\n", ip_address);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ip_address);

    // create raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0){
        printf("socket failed\n");
        exit(1);
    }

    if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0){
        printf("setsockopt failed\n");
        exit(1);
    }
    char* data = (char *)malloc(1000);
    strcpy(data, "Hello mF**k*r!");
    char* ip_packet = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(data));
    // struct iphdr *ip = (struct iphdr *)malloc(sizeof(struct iphdr));
    struct iphdr *ip = (struct iphdr *)ip_packet;
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0; // type of service (normal service)
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(data);
    ip->id = getpid();
    ip->frag_off = 0;       
    ip->ttl = 128;
    ip->protocol = IPPROTO_ICMP;
    ip->check = 0;
    ip->saddr = INADDR_ANY;
    ip->daddr = addr.sin_addr.s_addr;
    ip->check = checksum(ip, sizeof(struct iphdr));

    // struct icmphdr *icmp = (struct icmphdr *)malloc(sizeof(struct icmphdr));
    struct icmphdr *icmp = (struct icmphdr *)(ip_packet + sizeof(struct iphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = 123;
    icmp->un.echo.sequence = 0;
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr));
    printICMP(icmp);
    icmp->checksum = 0;
    memcpy(ip_packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, strlen(data));
    icmp->checksum = checksum(ip_packet + sizeof(struct iphdr), sizeof(struct icmphdr) + strlen(data));
    printICMP(icmp);
    
    // struct icmp *icmp2 = (struct icmp *)malloc(sizeof(struct icmp));
    // icmp2->icmp_type = ICMP_ECHO;
    // icmp2->icmp_code = 0;
    // icmp2->icmp_cksum = 0;
    // icmp2->icmp_id = getpid();
    // icmp2->icmp_seq = 0;
    // icmp2->icmp_cksum = checksum(icmp2, sizeof(struct icmp));
    // icmp2->icmp_data = 'A';

    // char *packet = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(data));
    
    // memcpy(packet, ip, sizeof(struct iphdr));
    // memcpy(packet + sizeof(struct iphdr), icmp, sizeof(struct icmphdr));
    // memcpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, strlen(data));

    // icmp->checksum = checksum(packet+sizeof(struct iphdr), sizeof(struct icmphdr) + strlen(data));
    // printf("checksum cal: %d\n", icmp->checksum);
    // // memcpy(packet + sizeof(struct iphdr), icmp, sizeof(struct icmphdr));
    // // memcpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, strlen(data));

    printf("IP----\n\n");
    printIP(ip);
    printf("ICMP----\n\n");
    printICMP(icmp);

    if (sendto(sock, ip_packet, ip->tot_len, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        printf("sendto failed\n");
        exit(1);
    }
    printf("Packet sent\n");
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);
    // char *recv_packet = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    char *recv_packet = (char *)malloc(1000);
    // if (recvfrom(sock, recv_packet, sizeof(struct iphdr) + sizeof(struct icmphdr), 0, (struct sockaddr *)&recv_addr, &recv_addr_len) < 0){
    //     printf("recvfrom failed\n");
    //     exit(1);
    // }   
    int recv_status = recvfrom(sock, recv_packet, 1000, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
    if(recv_status < 0){
        printf("recvfrom failed\n");
        exit(1);
    }
    printf("recv_status: %d\n", recv_status);
    printIP((struct iphdr *)recv_packet);
    printf("ICMP----\n\n");
    printICMP((struct icmphdr *)(recv_packet + sizeof(struct iphdr)));
    char* recv_data = (char *)malloc(1000);
    recv_data = recv_packet + sizeof(struct iphdr) + sizeof(struct icmphdr);
    printf("Received data: %s\n", recv_data);
    // for(int i=0; i<500; i++){
    //     printf("*%c*", recv_data[i]);
    // }
    // printf("\n");
    // printf("length of recv_packet: %d\n", strlen(recv_packet));

    // printf("Received packet from %s\n%s\n", inet_ntoa(recv_addr.sin_addr), recv_packet);

}