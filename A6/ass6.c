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

#define BUFSIZE 1500
#define MAXTTL 30
#define MAXWAIT 5

u_short in_cksum(u_short *addr, int len);

int main(int argc, char *argv[])
{
    int sockfd, n;
    char sendbuf[BUFSIZE], recvbuf[BUFSIZE];
    struct ip *ip;
    struct icmp *icmp;
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

    if ((hp = gethostbyname("localhost")) == NULL) {
        fprintf(stderr, "gethostbyname error: %s", hstrerror(h_errno));
    
            exit(1);
        }
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    bcopy(hp->h_addr, &dest.sin_addr, hp->h_length);

    printf("dest_size:%lu\n",sizeof(dest));
    printf("dest_family:%d\n",dest.sin_family);
    //printf("dest.sin_addr: %d\n",dest.sin_addr );

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        fprintf(stderr, "socket error: %s", strerror(errno));
        exit(1);
    }
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        fprintf(stderr, "setsockopt error: %s", strerror(errno));
        exit(1);
    }

    while (!done) {
        bzero(sendbuf, BUFSIZE);
        ip = (struct ip *) sendbuf;
        icmp = (struct icmp *) (sendbuf + sizeof(struct ip));
        
        ip->ip_v = IPVERSION;
        ip->ip_hl = sizeof(struct ip) >> 2;
        ip->ip_tos = 0;
        ip->ip_len = htons(sizeof(struct ip) + sizeof(struct icmp));
        ip->ip_id = htons(0);
        ip->ip_off = 0;
        ip->ip_ttl = ttl;
        ip->ip_p = IPPROTO_ICMP;
        ip->ip_sum = 0;
        ip->ip_src.s_addr = 0;
        ip->ip_dst = dest.sin_addr;

        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_id = htons(getpid());
        icmp->icmp_seq = htons(seq++);
        icmp->icmp_cksum = 0;
        icmp->icmp_cksum = in_cksum((u_short *) icmp, (int)sizeof(struct icmp));

        printf("sockfd: $%d$ \n",sockfd);
        printf("dest_size:%lu\n",sizeof(dest));
        printf("icmp size:%lu\n",sizeof(struct icmp));
        printf("ip size:%lu\n",sizeof(struct ip));

        printf("dest_family:%d\n",dest.sin_family);
        if (sendto(sockfd, sendbuf, (size_t)48, 0, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
            fprintf(stderr, "sendto error: %s", strerror(errno));
            exit(1);
        }
        tv.tv_sec = MAXWAIT;
        tv.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);
        if ((n = select(sockfd + 1, &rset, NULL, NULL, &tv)) < 0) {
            fprintf(stderr, "select error: %s", strerror(errno));
            exit(1);
        } else if (n == 0) {
            printf("%d: * * *\r", ttl);
        } else {
            fromlen = sizeof(from);
            if ((n = recvfrom(sockfd, recvbuf, BUFSIZE, 0, (struct sockaddr *) &from, &fromlen)) < 0) {
                fprintf(stderr, "recvfrom error: %s", strerror(errno));
                exit(1);
            }
            ip = (struct ip *) recvbuf;
            icmp = (struct icmp *) (recvbuf + (ip->ip_hl << 2));
            if (icmp->icmp_type == ICMP_ECHOREPLY) {
                printf("%d: %s\r", ttl, inet_ntoa(from.sin_addr));
                done = 1;
            } else {
                printf("%d: %s\r", ttl, inet_ntoa(from.sin_addr));
            }
        }
        ttl++;
        if (ttl > MAXTTL) {
            fprintf(stderr, "ttl > MAXTTL");
            exit(1);
        }
    }
    exit(0);
}

u_short in_cksum(u_short *addr, int len)
{
    int nleft = len;
    u_short *w = addr;
    u_short answer;
    int sum = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(u_char *) (&answer) = *(u_char *) w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}
