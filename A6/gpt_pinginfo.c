#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PACKET_SIZE 64
#define MAX_HOPS 64
#define MAX_TRIES 3

struct icmp_packet
{
    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned short id;
    unsigned short sequence;
    struct timeval timestamp;
};

struct ip_packet
{
    unsigned char version_and_length;
    unsigned char type_of_service;
    unsigned short total_length;
    unsigned short identification;
    unsigned short flags_and_fragment_offset;
    unsigned char time_to_live;
    unsigned char protocol;
    unsigned short header_checksum;
    unsigned int source_address;
    unsigned int destination_address;
};

unsigned short calculate_checksum(unsigned short *data, int length)
{
    unsigned int sum = 0;
    unsigned short result;

    while (length > 1)
    {
        sum += *data++;
        length -= 2;
    }

    if (length == 1)
    {
        sum += *(unsigned char *)data;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

void print_address(unsigned int address)
{
    char buffer[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &address, buffer, INET_ADDRSTRLEN);

    printf("%s", buffer);
}

void print_elapsed_time(struct timeval *start_time)
{
    struct timeval current_time;
    long elapsed_time;

    gettimeofday(&current_time, NULL);

    elapsed_time = (current_time.tv_sec - start_time->tv_sec) * 1000000 + (current_time.tv_usec - start_time->tv_usec);

    printf("%.3f ms", elapsed_time / 1000.0);
}

void ping(char *hostname)
{
    int ttl = 1;
    int tries = 0;
    int done = 0;
    int sockfd;
    struct addrinfo hints, *result, *rp;
    struct icmp_packet icmp_send_packet, icmp_recv_packet;
    struct ip_packet ip_recv_packet;
    struct timeval start_time;
    struct timeval timeout;
    fd_set read_set;
    char buffer[PACKET_SIZE];
    int bytes_received;
    int bytes_sent;
    unsigned int source_address;
    unsigned int dest_address;
    double elapsed_time;
    double estimated_bandwidth;
    int num_hops = 0;

    // Resolve hostname to IP address
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    if (getaddrinfo(hostname, NULL, &hints, &result) != 0)
    {
        printf("Error resolving hostname: %s\n", strerror(errno));
        return;
    }

    // Try each IP address until we successfully create a socket and connect
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

        if (sockfd == -1)
        {
            continue;
        }

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
        {
            break;
        }
    }

    if (rp == NULL)
    {
        printf("Error connecting socket: %s\n", strerror(errno));

        return;
    }

    source_address = ((struct sockaddr_in *)(rp->ai_addr))->sin_addr.s_addr;
    dest_address = ((struct sockaddr_in *)(rp->ai_addr))->sin_addr.s_addr;

    freeaddrinfo(result);

    // Set TTL on socket
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1)
    {
        printf("Error setting TTL: %s\n", strerror(errno));
        return;
    }

    // Set timeout on socket
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        printf("Error setting timeout: %s\n", strerror(errno));
        return;
    }

    // Initialize ICMP packet
    icmp_send_packet.type = 8; // Echo Request
    icmp_send_packet.code = 0;
    icmp_send_packet.id = htons(getpid());
    icmp_send_packet.sequence = htons(1);

    // Send ICMP packets with increasing TTL until we receive a response or hit the maximum number of hops
    while (!done && num_hops < MAX_HOPS)
    {
        // Set TTL on socket
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1)
        {
            printf("Error setting TTL: %s\n", strerror(errno));
            return;
        }

        // Initialize IP packet
        memset(&ip_recv_packet, 0, sizeof(ip_recv_packet));
        ip_recv_packet.version_and_length = 0x45;
        ip_recv_packet.type_of_service = 0;
        ip_recv_packet.total_length = htons(PACKET_SIZE);
        ip_recv_packet.identification = htons(getpid());
        ip_recv_packet.time_to_live = ttl;
        ip_recv_packet.protocol = IPPROTO_ICMP;
        ip_recv_packet.source_address = source_address;
        ip_recv_packet.destination_address = dest_address;
        ip_recv_packet.header_checksum = calculate_checksum((unsigned short *)&ip_recv_packet, sizeof(ip_recv_packet));

        // Initialize ICMP packet
        icmp_send_packet.checksum = 0;
        icmp_send_packet.timestamp = start_time;
        icmp_send_packet.checksum = calculate_checksum((unsigned short *)&icmp_send_packet, sizeof(icmp_send_packet));

        // Send ICMP packet
        bytes_sent = send(sockfd, &icmp_send_packet, sizeof(icmp_send_packet), 0);

        if (bytes_sent == -1)
        {
            printf("Error sending packet: %s\n", strerror(errno));
            return;
        }

        // Wait for response
        tries = 0;

        while (tries < MAX_TRIES)
        {
            FD_ZERO(&read_set);
            FD_SET(sockfd, &read_set);

            bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);

            if (bytes_received == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    tries++;
                    continue;
                }
                else
                {
                    printf("Error receiving packet: %s\n", strerror(errno));
                    return;
                }
            }

            // Parse IP header
            memcpy(&ip_recv_packet, buffer, sizeof(ip_recv_packet));

            // Parse ICMP header
            memcpy(&icmp_recv_packet, buffer + sizeof(ip_recv_packet), sizeof(icmp_recv_packet));

            // Check if response is for our packet
            if (icmp_recv_packet.id == icmp_send_packet.id && icmp_recv_packet.sequence == icmp_send_packet.sequence)
            {
                done = 1;
                gettimeofday(&start_time, NULL);
                //elapsed_time = (double)(start_time.tv_sec + end_time.tv_sec) * 1000.0; ///////
                //elapsed_time += (double)(start_time.tv_usec - end_time.tv_usec) / 1000.0;
                elapsed_time = (double)(start_time.tv_sec) * 1000.0;
                printf("%d.\t%s\t%.3f ms\n", num_hops, inet_ntoa(*(struct in_addr *)&ip_recv_packet.source_address), elapsed_time);
                break;
            }

            tries++;
        }

        num_hops++;
        ttl++;
    }

    close(sockfd);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    printf("Tracing route to %s\n", argv[1]);

    ping(argv[1]);

    return 0;
}
