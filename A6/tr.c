// trace iproute to destination host using RAW sockets
// compile: gcc -o a6 a6.c
// run: sudo ./a6

// sudo ./a6 www.google.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#define BUFSIZE 1500
#define MAXWAIT 5
const int MAX_TTL = 10;
struct timeval start_time, end_time;


uint16_t in_cksum(uint16_t *addr, int len);

void printIP(struct iphdr *ip);

struct iphdr *createIPHeader(char *mssg, char *sendbuf, int ttl, struct sockaddr_in *dest);
struct icmphdr *createICMPHeader(char *mssg, char *sendbuf);

double timeval_diff(struct timeval *start, struct timeval *end)
{
    return (double)(end->tv_sec - start->tv_sec) * 1000000 + (double)(end->tv_usec - start->tv_usec);
}

struct icmphdr * send_recv(int sockfd, 
               char *sendbuf, 
               char *recvbuf,
               struct iphdr *ip,
               struct icmphdr *icmp,
               int *print_flag,
               struct sockaddr_in *dest, 
               struct sockaddr_in *from,
               socklen_t fromlen,
               struct timeval *tv,
               fd_set *rset,
               int ttl,
               double *rtt
            )
{
    gettimeofday(&start_time, NULL);

    int n;
    if (n = sendto(sockfd, sendbuf, ip->tot_len, 0, (struct sockaddr *)dest, sizeof(*dest)) < 0)
    {
        fprintf(stderr, "sendto error: %s", strerror(errno));
        exit(1);
    }

    tv->tv_sec = MAXWAIT;
    tv->tv_usec = 0;
    FD_ZERO(rset);
    FD_SET(sockfd, rset);

    if ((n = select(sockfd + 1, rset, NULL, NULL, tv)) < 0)
    {
        fprintf(stderr, "select error: %s", strerror(errno));
        exit(1);
    }
    else if (n == 0)
    {
        printf("%d: *\n", ttl);
    }
    else
    {
        recvbuf = (char *)malloc(BUFSIZE);
        memset(recvbuf, 0, BUFSIZE);

        fromlen = sizeof(from);
        if ((n = recvfrom(sockfd, recvbuf, BUFSIZE, 0, (struct sockaddr *)from, &fromlen)) < 0)
        {
            fprintf(stderr, "recvfrom error: %s", strerror(errno));
            exit(1);
        }

        gettimeofday(&end_time, NULL);
        *rtt = timeval_diff(&start_time, &end_time);

        ip = (struct iphdr *)recvbuf;
        icmp = (struct icmphdr *)(recvbuf + sizeof(struct iphdr));
        char *recv_mssg = (char *)(recvbuf + sizeof(struct iphdr) + sizeof(struct icmphdr));
    
        // printf("recv_mssg: %s\n", recv_mssg);
        *print_flag = 1;
    }

    return icmp;
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    char *sendbuf, *recvbuf;
    struct iphdr *ip;
    struct icmphdr *icmp;
    struct sockaddr_in dest, from;
    socklen_t fromlen;
    struct hostent *hp;
    struct timeval tv;
    fd_set rset;
    int on = 1;
    int seq = 0;
    int ttl = 1;
    int done = 0;
    int i;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s hostname", argv[0]);
        exit(1);
    }

    if ((hp = gethostbyname(argv[1])) == NULL)
    {
        fprintf(stderr, "gethostbyname error: %s", hstrerror(h_errno));

        exit(1);
    }
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    struct in_addr **addr_list = (struct in_addr **)hp->h_addr_list;
    bcopy(addr_list[0], &dest.sin_addr, hp->h_length);

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        fprintf(stderr, "socket error: %s", strerror(errno));
        exit(1);
    }
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0)
    {
        fprintf(stderr, "setsockopt error: %s", strerror(errno));
        exit(1);
    }

    char *mssg;
    char *prev_ip;
    prev_ip = (char *)malloc(100);
    memset(prev_ip, 0, 100);
    strcpy(prev_ip, "110.117.2.96");

    int print_flag = 0;
    printf("#hops\t\tIP Address\t\t\tLatency\t\t\tBandwidth\n");
    while (!done)
    {
        mssg = (char *)malloc(100);
        strcpy(mssg, "Hello World!!");
        int mssg_len = strlen(mssg);
        // printf("mssg: %s\n", mssg);

        sendbuf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg));
        ip = createIPHeader(mssg, sendbuf, ttl, &dest);
        icmp = createICMPHeader(mssg, sendbuf);

        icmp->checksum = 0;
        memcpy(sendbuf + sizeof(struct iphdr) + sizeof(struct icmphdr), mssg, strlen(mssg));
        icmp->checksum = in_cksum((uint16_t *)(sendbuf + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(mssg));

        // printIP(ip);
        double rtt1;
        icmp = send_recv(sockfd, sendbuf, recvbuf, ip, icmp, &print_flag, &dest, &from, fromlen, &tv, &rset, ttl, &rtt1);

        
        
        
        
        memset(mssg, 0, 100);
        strcpy(mssg, "");
        // printf("mssg: %s\n", mssg);

        sendbuf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg));
        ip = createIPHeader(mssg, sendbuf, ttl, &dest);
        icmp = createICMPHeader(mssg, sendbuf);

        icmp->checksum = 0;
        memcpy(sendbuf + sizeof(struct iphdr) + sizeof(struct icmphdr), mssg, strlen(mssg));
        icmp->checksum = in_cksum((uint16_t *)(sendbuf + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(mssg));

        // printIP(ip);
        double rtt;
        icmp = send_recv(sockfd, sendbuf, recvbuf, ip, icmp, &print_flag, &dest, &from, fromlen, &tv, &rset, ttl, &rtt);


        if (print_flag == 1)
        {
            rtt1= abs(rtt1- rtt)/2000.0;
            double bandwidth = (mssg_len * 8) / rtt1;
            bandwidth/=1000.0;
            if (icmp->type == ICMP_ECHOREPLY)
            {
                // printf("%d: %s\n",ttl,inet_ntoa(from.sin_addr));
                
                printf("%d\t%s -> %s\t\t%.3f ms\t\t%.3f Mbps\n", ttl ,prev_ip, inet_ntoa(from.sin_addr), rtt/2000.0,bandwidth);
                done = 1;
            }
            else
            {
                printf("%d\t%s -> %s\t\t%.3f ms\t\t%.3f Mbps\n", ttl, prev_ip, inet_ntoa(from.sin_addr),rtt/2000.0,bandwidth);
                memset(prev_ip, 0, 100);
                strcpy(prev_ip, inet_ntoa(from.sin_addr));
            }
        }

        ttl++;
        if (ttl > MAX_TTL)
        {
            printf("ttl > MAX_TTL\n");
            fprintf(stderr, "ttl > MAX_TTL");
            exit(1);
        }
    }
    exit(0);
}

uint16_t in_cksum(uint16_t *addr, int len)
{
    int nleft = len;
    uint16_t *w = addr;
    uint16_t answer;
    int sum = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(uint16_t *)(&answer) = *(uint16_t *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
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

struct iphdr *createIPHeader(char *mssg, char *sendbuf, int ttl, struct sockaddr_in *dest)
{
    struct iphdr *ip = (struct iphdr *)sendbuf;
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0; // type of service (normal service)
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg);
    ip->id = getpid();
    ip->frag_off = 0;
    ip->ttl = ttl;
    ip->protocol = IPPROTO_ICMP;
    ip->check = 0;
    ip->saddr = INADDR_ANY;
    ip->daddr = dest->sin_addr.s_addr;
    ip->check = in_cksum((uint16_t *)ip, sizeof(struct iphdr));

    return ip;
}

struct icmphdr *createICMPHeader(char *mssg, char *sendbuf)
{
    struct icmphdr *icmp = (struct icmphdr *)(sendbuf + sizeof(struct iphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = 123;
    icmp->un.echo.sequence = 0;
    icmp->checksum = in_cksum((uint16_t *)icmp, (int)sizeof(struct icmphdr));
    return icmp;
}