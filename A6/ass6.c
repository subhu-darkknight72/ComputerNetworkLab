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

#define BUFSIZE 1500
#define MAXWAIT 5
const int MAX_TTL=5;

uint16_t in_cksum(uint16_t *addr, int len);

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

    if (argc != 2) {
        fprintf(stderr, "usage: %s hostname", argv[0]);
        exit(1);
    }

    if ((hp = gethostbyname(argv[1] )) == NULL)
    {
        fprintf(stderr, "gethostbyname error: %s", hstrerror(h_errno));

        exit(1);
    }
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    struct in_addr **addr_list = (struct in_addr **)hp->h_addr_list;
    bcopy(addr_list[0], &dest.sin_addr, hp->h_length);

    printf("dest_size:%lu\n", sizeof(dest));
    printf("dest_family:%d\n", dest.sin_family);
    // printf("dest.sin_addr: %d\n",dest.sin_addr );

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
    while (!done)
    {
        printf("\n~~~~~Enter While~~~~~\n");

        mssg = (char *)malloc(100);
        strcpy(mssg, "GG <3, always");
        printf("mssg: %s\n", mssg);

        sendbuf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg));
        ip = (struct iphdr *)sendbuf;
        icmp = (struct icmphdr *)(sendbuf + sizeof(struct iphdr));

        ip->version = 4;
        ip->ihl = 5;
        ip->tos = 0; // type of service (normal service)
        ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg);
        ip->id = getpid();
        ip->frag_off = 0;
        ip->ttl = 128;
        ip->protocol = IPPROTO_ICMP;
        ip->check = 0;
        ip->saddr = INADDR_ANY;
        ip->daddr = dest.sin_addr.s_addr;
        ip->check = in_cksum((uint16_t *)ip, sizeof(struct iphdr));

        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.id = 123;
        icmp->un.echo.sequence = 0;
        icmp->checksum = in_cksum((uint16_t *)icmp, (int)sizeof(struct icmphdr));

        icmp->checksum = 0;
        memcpy(sendbuf + sizeof(struct iphdr) + sizeof(struct icmphdr), mssg, strlen(mssg));
        icmp->checksum = in_cksum((uint16_t *)(sendbuf + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(mssg));

        
        printf("ip total length: %d\n", ip->tot_len);
        printf("dest_family:%d\n", dest.sin_family);
        printIP(ip);

        if (n = sendto(sockfd, sendbuf, ip->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
        {
            fprintf(stderr, "sendto error: %s", strerror(errno));
            exit(1);
        }
        else
        {
            printf("...... sendto success ......\n");
            printf("sizeof(sendbuf): %lu\n", sizeof(sendbuf));
            printf("send status: %d\n", n);
        }
        
        tv.tv_sec = MAXWAIT;
        tv.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);
        
        if ((n = select(sockfd + 1, &rset, NULL, NULL, &tv)) < 0)
        {
            fprintf(stderr, "select error: %s", strerror(errno));
            exit(1);
        }
        else if (n == 0)
        {
            printf("%d: * * *\r", ttl);
        }
        else
        {
            recvbuf = (char *)malloc(BUFSIZE);
            memset(recvbuf, 0, BUFSIZE);

            fromlen = sizeof(from);
            if ((n = recvfrom(sockfd, recvbuf, BUFSIZE, 0, (struct sockaddr *)&from, &fromlen)) < 0)
            {
                fprintf(stderr, "recvfrom error: %s", strerror(errno));
                exit(1);
            }
            else
            {
                printf("...... recvfrom success ......\n");
                printf("recv status: %d\n", n);
            }

            ip = (struct iphdr *)recvbuf;
            icmp = (struct icmphdr *)(recvbuf + sizeof(struct iphdr));
            char *recv_mssg = (char *)(recvbuf 
                                     + sizeof(struct iphdr) 
                                     + sizeof(struct icmphdr));
            printf("recv_mssg: %s\n", recv_mssg);

            if (icmp->type == ICMP_ECHOREPLY)
            {
                printf("%d: %s\r", ttl, inet_ntoa(from.sin_addr));
                done = 1;
            }
            else
            {
                printf("%d: %s\r", ttl, inet_ntoa(from.sin_addr));
            }
        }
        ttl++;
        if (ttl > MAX_TTL)
        {
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
