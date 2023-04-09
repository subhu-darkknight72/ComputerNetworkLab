// 20CS30019: Gitanjali Gupta
// 20CS10064: Subhajyoti Halder

// compile: gcc -o pingnetinfo pingnetinfo.c
// run: sudo ./pingnetinfo iitkgp.ac.in 5 1 64

// run: sudo ./pingnetinfo <server> <no. of probes per hop> <propbe delay> <max no. of hops>

//  Makefile for testing the code:
/*
iitkgp:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti iitkgp.ac.in 5 0 64

local:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti localhost

moodle:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti kgpmoodlenew.iitkgp.ac.in 5 1 10

csemoodle:
	sudo gcc -o pgti pingnetinfo.c
	sudo ./pgti moodlecse.iitkgp.ac.in 4 2

gitu:
	gcc -o ass ass6.c 
	sudo ./ass iitkgp.ac.in

default:
	traceroute iitkgp.ac.in

push:
	git add -A
	git commit -m "GG <3"
	git push

clean:
	sudo rm -f *.o localhost pgti ass pingnetinfo pingnetinfo_output.txt

*/

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
#include <linux/tcp.h>
#include <linux/udp.h>

#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#define BUFSIZE 1500
#define MAXWAIT 5
int MAX_TTL = 10;
struct timeval start_time, end_time;
FILE *file;


uint16_t in_cksum(uint16_t *addr, int len);

void printIP(struct iphdr *ip);
void printICMP(struct icmphdr *icmp);

struct iphdr *createIPHeader(char *mssg, char *sendbuf, int ttl, struct sockaddr_in *dest);
struct icmphdr *createICMPHeader(char *mssg, char *sendbuf);

double timeval_diff(struct timeval *start, struct timeval *end);

struct icmphdr *send_recv(int sockfd,
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
                          double *rtt);

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
    int iter_count = 5;
    int T=1;

    file = fopen("pingnetinfo_output.txt", "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    if (argc < 2)
    {
        fprintf(stderr, "usage: %s hostname", argv[0]);
        exit(1);
    }

    if (argc >= 3)
        iter_count = atoi(argv[2]);
    if (argc >= 4)
        T = atoi(argv[3]);
    if (argc >= 5)
        MAX_TTL = atoi(argv[4]);


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

    int mssg_len;
    char *mssg;
    char *prev_ip;
    prev_ip = (char *)malloc(100);
    memset(prev_ip, 0, 100);
    strcpy(prev_ip, "110.117.2.96");

    int print_flag = 0;
    printf("NOTE: The IP Headers and ICMP Headers are printed in the output file.\n");
    printf("Output file: pingnetinfo_output.txt\n");
    printf("#probes: %d, probe interval: %dsec\n", iter_count, T);
    printf("traceroute to %s (%s), %d hops max\n", argv[1], inet_ntoa(dest.sin_addr), MAX_TTL);
    printf("----------------------------------------------------------------------------------------------\n");
    printf("#hops\t|\t\tIP Address\t\t|\tLatency\t\t|\tBandwidth    |\n");
    printf("----------------------------------------------------------------------------------------------\n");
    while (!done)
    {
        fprintf(file,"\n======================================================================");
        fprintf(file,"\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HOP:%d~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n",ttl);
        fprintf(file,"======================================================================\n");
        // ~~~~~~~~~~~~ check for correct Hop-IP ~~~~~~~~~~~~
        char *curr_ip, *temp_ip;
        curr_ip = (char *)malloc(100);
        temp_ip = (char *)malloc(100);

        int i=0, drop_cnt=0;
        while(i<5){
            mssg = (char *)malloc(100);
            strcpy(mssg, "AGkMKB");

            sendbuf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg));
            ip = createIPHeader(mssg, sendbuf, ttl, &dest);
            icmp = createICMPHeader(mssg, sendbuf);

            icmp->checksum = 0;
            memcpy(sendbuf + sizeof(struct iphdr) + sizeof(struct icmphdr), mssg, strlen(mssg));
            icmp->checksum = in_cksum((uint16_t *)(sendbuf + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(mssg));

            double rtt0;
            icmp = send_recv(sockfd, sendbuf, recvbuf, ip, icmp, &print_flag, &dest, &from, fromlen, &tv, &rset, ttl, &rtt0);

            if(i==0)
                strcpy(curr_ip, inet_ntoa(from.sin_addr));
            else{
                strcpy(temp_ip, inet_ntoa(from.sin_addr));
                if(strcmp(curr_ip, temp_ip) != 0){
                    strcpy(curr_ip, temp_ip);
                    // printf("Restarted %s %s\n", curr_ip, temp_ip);
                    i = 0;
                    if(drop_cnt++ > 5){
                        printf("Dropped 5-times. Exiting...\n");
                        exit(1);
                    }
                    sleep(1);
                    continue;
                }
            }
            i++;
        }


        double rtt1=0, rtt=0;
        for(int i=0; i<iter_count; i++){

            // ~~~~~~~~~~~~ Send Message ~~~~~~~~~~~~
            mssg = (char *)malloc(100);
            strcpy(mssg, "Hello World!!");
            mssg_len = strlen(mssg);

            sendbuf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg));
            ip = createIPHeader(mssg, sendbuf, ttl, &dest);
            icmp = createICMPHeader(mssg, sendbuf);

            icmp->checksum = 0;
            memcpy(sendbuf + sizeof(struct iphdr) + sizeof(struct icmphdr), mssg, strlen(mssg));
            icmp->checksum = in_cksum((uint16_t *)(sendbuf + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(mssg));

            double rtt_1;
            icmp = send_recv(sockfd, sendbuf, recvbuf, ip, icmp, &print_flag, &dest, &from, fromlen, &tv, &rset, ttl, &rtt_1);
            rtt1 += rtt_1; 


            // ~~~~~~~~~~~~ NULL Message ~~~~~~~~~~~~
            memset(mssg, 0, 100);
            strcpy(mssg, "");
            // printf("mssg: %s\n", mssg);

            sendbuf = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen(mssg));
            ip = createIPHeader(mssg, sendbuf, ttl, &dest);
            icmp = createICMPHeader(mssg, sendbuf);

            icmp->checksum = 0;
            memcpy(sendbuf + sizeof(struct iphdr) + sizeof(struct icmphdr), mssg, strlen(mssg));
            icmp->checksum = in_cksum((uint16_t *)(sendbuf + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(mssg));

            double rtt_2;
            icmp = send_recv(sockfd, sendbuf, recvbuf, ip, icmp, &print_flag, &dest, &from, fromlen, &tv, &rset, ttl, &rtt_2);
            rtt = rtt + rtt_2;

            // wait for T seconds
            sleep(T);
        }

        rtt /= iter_count;
        rtt1 /= iter_count;

        // ~~~~~~~~~~~~ Print ~~~~~~~~~~~~
        if (print_flag == 1)
        {
            rtt1 = abs(rtt1 - rtt) / 2000.0;
            double bandwidth = (mssg_len * 8) / rtt1;
            bandwidth /= 1000.0;

            // convert double to string
            char bw[10];
            sprintf(bw, "%.3f", bandwidth);


            printf("  %d\t|%15s   -> %15s\t|\t%.3f ms\t|\t%7s Mbps |\n", ttl, prev_ip, inet_ntoa(from.sin_addr), rtt / 2000.0, bw);
            if (icmp->type == ICMP_ECHOREPLY)
                done = 1;
            else
            {
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
    printf("----------------------------------------------------------------------------------------------\n");
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
    fprintf(file,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(file,"|  Version:%-2d |  IHL:%-4d  |   TOS:%-2d  |   TotalLength:%-4d     |\n", ip->version, ip->ihl, ip->tos, ip->tot_len);
    fprintf(file,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(file,"|   Identification:%-6d   |%d|%d|%d|  FragmentationOffset:%-4d   |\n", ntohs(ip->id), ip->frag_off && (1 << 15), ip->frag_off && (1 << 14), ip->frag_off && (1 << 14), ip->frag_off);
    fprintf(file,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(file,"|    TTL:%-4d   |  Protocol:%-2d  |        IPChecksum:%-6d      |\n", ip->ttl, ip->protocol, ip->check);
    fprintf(file,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(file,"|                Source Address:%-16s                |\n", inet_ntoa(*(struct in_addr *)&ip->saddr));
    fprintf(file,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(file,"|             Destination Address:%-16s              |\n", inet_ntoa(*(struct in_addr *)&ip->daddr));
    fprintf(file,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void printICMP(struct icmphdr *icmp)
{
    fprintf(file, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(file, "|    Type:%-2d    |    Code:%-2d    |      ICMPchecksum:%-6d      |\n", icmp->type, icmp->code, icmp->checksum);
    fprintf(file, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    if (icmp->type == ICMP_ECHO || icmp->type == ICMP_ECHOREPLY)
        fprintf(file, "|       Identifier:%-6d       |      SequenceNumber:%-6d    |\n", icmp->un.echo.id, icmp->un.echo.sequence);
    fprintf(file, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void printTCP(const struct tcphdr *tcp)
{
    fprintf(file, "-----------------------------------------------------------------\n");
    fprintf(file, "|         source:%-6d         |          dest:%-6d          |\n", ntohs(tcp->source), ntohs(tcp->dest));
    fprintf(file, "-----------------------------------------------------------------\n");
    fprintf(file, "|                       sequence:%-8d                       |\n", tcp->seq);
    fprintf(file, "|                         ack:%-8d                          |\n", tcp->ack_seq);
    fprintf(file, "-----------------------------------------------------------------\n");
    fprintf(file, "| hlen:%-4d|reserved|%d|%d|%d|%d|%d|%d|          rwnd:%-6d          |\n", tcp->doff, tcp->urg, tcp->ack, tcp->psh, tcp->rst, tcp->syn, tcp->fin, tcp->window);
    fprintf(file, "-----------------------------------------------------------------\n");
    fprintf(file, "|          check:%-6d         |         urgptr:%-6d         |\n", tcp->check, tcp->urg_ptr);
    fprintf(file, "-----------------------------------------------------------------file");
}

void printUDP(const struct udphdr *udp)
{
    fprintf(file, "-----------------------------------------------------------------\n");
    fprintf(file, "|         source:%-6d         |          dest:%-6d          |\n", ntohs(udp->source), ntohs(udp->dest));
    fprintf(file, "-----------------------------------------------------------------\n");
    fprintf(file, "|           len:%-6d          |          check:%-6d         |\n", ntohs(udp->len), udp->check);
    fprintf(file, "-----------------------------------------------------------------\n");
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

double timeval_diff(struct timeval *start, struct timeval *end)
{
    return (double)(end->tv_sec - start->tv_sec) * 1000000 + (double)(end->tv_usec - start->tv_usec);
}

struct icmphdr *send_recv(int sockfd,
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
                          double *rtt)
{
    gettimeofday(&start_time, NULL);

    int n;
    if (n = sendto(sockfd, sendbuf, ip->tot_len, 0, (struct sockaddr *)dest, sizeof(*dest)) < 0)
    {
        fprintf(stderr, "sendto error: %s", strerror(errno));
        exit(1);
    }
    
    fprintf(file,"\n~~~~~~~~~~~~~~~~~~~~~~~~Send IP Header~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printIP(ip);
    fprintf(file,"\n\n~~~~~~~~~~~~~~~~~~~~~~~Send ICMP Header~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printICMP(icmp);

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
        printf("  %d\t *\n", ttl);
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

        fprintf(file,"\n\n~~~~~~~~~~~~~~~~~~~~~~~~Recieve IP Header~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printIP(ip);
        fprintf(file,"\n\n~~~~~~~~~~~~~~~~~~~~~~~Recieve ICMP Header~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printICMP(icmp);

        // printf("recv_mssg: %s\n", recv_mssg);
        *print_flag = 1;
    }

    return icmp;
}

