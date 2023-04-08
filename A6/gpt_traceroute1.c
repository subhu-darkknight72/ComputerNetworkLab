
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/time.h>

#define PACKET_SIZE 64
#define MAX_HOPS 30

float get_latency(int rtt_0bytes)
{
    return (rtt_0bytes / 2);
}
float get_bandwidth(int diff_bytes, int diff_rtt)
{
    float temp = 2 * diff_bytes;
    temp /= diff_rtt;
    return temp;
}

// Structure for storing information about each hop
struct hop_info
{
    struct timeval rtt;     // Round-trip time
    struct in_addr ip_addr; // IP address
    int is_reached;         // Whether the hop is reached or not
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <hostname/IP>\n", argv[0]);
        exit(1);
    }

    struct hostent *host;
    struct sockaddr_in dest_addr;
    struct iphdr *iph;
    struct icmphdr *icmph;
    char packet[PACKET_SIZE];
    struct timeval timeout;
    int sockfd, i, recv_bytes, addr_len, ttl = 1, max_hops_reached = 0;
    struct hop_info hops[MAX_HOPS];

    // Resolve the hostname to IP address
    if ((host = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = 0;
    dest_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);

    memset(&packet, 0, sizeof(packet));

    // Create a raw socket for sending ICMP packets
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("socket");
        exit(1);
    }

    printf("Traceroute to %s (%s)\n", argv[1], inet_ntoa(dest_addr.sin_addr));

    while (ttl <= MAX_HOPS && !max_hops_reached)
    {
        // Set the TTL (Time-to-Live) of the packet
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        // Initialize the hop information
        hops[ttl - 1].is_reached = 0;
        hops[ttl - 1].rtt.tv_sec = 0;
        hops[ttl - 1].rtt.tv_usec = 0;

        printf("%d. ", ttl);
        fflush(stdout);

        // Send an ICMP Echo Request packet
        if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        {
            perror("sendto");
            exit(1);
        }

        // Set the timeout for receiving ICMP Echo Reply packet
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Wait for the ICMP Echo Reply packet
        if (select(sockfd + 1, &readfds, NULL, NULL, &timeout) > 0)
        {
            if (FD_ISSET(sockfd, &readfds))
            {
                addr_len = sizeof(dest_addr);
                if ((recv_bytes = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, (socklen_t *)&addr_len)) < 0)
                {
                    perror("recvfrom");
                    exit(1);
                }

                iph = (struct iphdr *)packet;
                icmph = (struct icmphdr *)(packet + (iph->ihl << 2));
                printf("icmp->type:%d\n",icmph->type);
                // Check if the received packet is an ICMP Echo Reply
                if (icmph->type == ICMP_ECHOREPLY)
                {
                    hops[ttl - 1].is_reached = 1;
                    hops[ttl - 1].ip_addr = dest_addr.sin_addr;
                    gettimeofday(&hops[ttl - 1].rtt, NULL);

                    // Print the IP address and round-trip time of the hop
                    printf("%s (%s) %.2f ms\n", host->h_name, inet_ntoa(dest_addr.sin_addr),
                           hops[ttl - 1].rtt.tv_sec * 1000.0 + hops[ttl - 1].rtt.tv_usec / 1000.0);
                }
                else
                {
                    // Print the IP address of the intermediate hop
                    printf("%s\n", inet_ntoa(dest_addr.sin_addr));
                }

                // Check if the destination host is reached
                if (icmph->type == ICMP_ECHOREPLY || icmph->type == ICMP_DEST_UNREACH)
                {
                    max_hops_reached = 1;
                }
            }
        }
        else
        {
            // Print "*" if no reply received within timeout
            printf("*\n");
        }

        ttl++;
    }

    // Close the socket
    close(sockfd);

    // Print the summary of the traceroute
    printf("\nTraceroute complete. Summary:\n");
    for (i = 0; i < ttl - 1; i++)
    {
        if (hops[i].is_reached)
        {
            printf("%d. %s (%s) %.2f ms\n", i + 1, host->h_name, inet_ntoa(hops[i].ip_addr),
                   hops[i].rtt.tv_sec * 1000.0 + hops[i].rtt.tv_usec / 1000.0);
        }
        else
        {
            printf("%d. *\n", i + 1);
        }
    }

    return 0;
}