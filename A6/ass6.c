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
#define MAXTTL 30
#define MAXWAIT 5

uint16_t in_cksum(uint16_t *addr, int len);

int main(int argc, char *argv[])
{
    int sockfd, n;
    char sendbuf[BUFSIZE], recvbuf[BUFSIZE];
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

    // if (argc != 2) {
    //     fprintf(stderr, "usage: %s hostname", argv[0]);
    //     exit(1);
    // }

    if ((hp = gethostbyname("localhost")) == NULL)
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
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
    {
        fprintf(stderr, "setsockopt error: %s", strerror(errno));
        exit(1);
    }

    while (!done)
    {
        bzero(sendbuf, BUFSIZE);
        ip = (struct iphdr *)sendbuf;
        icmp = (struct icmphdr *)(sendbuf + sizeof(struct iphdr));

        ip->version = 4;
        ip->ihl = 5;
        ip->tos = 0; // type of service (normal service)
        ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(sendbuf);
        ip->id = getpid();
        ip->frag_off = 0;
        ip->ttl = 128;
        ip->protocol = IPPROTO_ICMP;
        ip->check = 0;
        ip->saddr = INADDR_ANY;
        ip->daddr = dest.sin_addr.s_addr;
        ip->check = checksum(ip, sizeof(struct iphdr));

        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.id = 123;
        icmp->un.echo.sequence = 0;
        icmp->checksum = in_cksum((uint16_t *)icmp, (int)sizeof(struct icmphdr));

        printf("sockfd: $%d$ \n", sockfd);
        printf("dest_size:%lu\n", sizeof(dest));
        printf("icmp size:%lu\n", sizeof(struct icmphdr));
        printf("ip size:%lu\n", sizeof(struct iphdr));

        printf("dest_family:%d\n", dest.sin_family);
        if (sendto(sockfd, sendbuf, sizeof(ip), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
        {
            fprintf(stderr, "sendto error: %s", strerror(errno));
            exit(1);
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
            fromlen = sizeof(from);
            if ((n = recvfrom(sockfd, recvbuf, BUFSIZE, 0, (struct sockaddr *)&from, &fromlen)) < 0)
            {
                fprintf(stderr, "recvfrom error: %s", strerror(errno));
                exit(1);
            }
            ip = (struct iphdr *)recvbuf;
            icmp = (struct icmphdr *)(recvbuf + (ip->ihl << 2));
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
        if (ttl > MAXTTL)
        {
            fprintf(stderr, "ttl > MAXTTL");
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
