#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define ICMP_ECHO_REQUEST 8

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in dest_addr;
    char buffer[BUF_SIZE];
    int packet_size, num_bytes;
    struct icmp *icmp_packet;

    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Resolve destination address
    // struct hostent *host = gethostbyname("google.co.in");
    struct hostent *host = gethostbyname("google.co.in");
    if (host == NULL) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    // Set destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = *((struct in_addr *) host->h_addr);

    // Prepare ICMP packet
    packet_size = sizeof(struct icmp);
    icmp_packet = (struct icmp *) buffer;
    icmp_packet->icmp_type = ICMP_ECHO_REQUEST;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_id = getpid() & 0xFFFF;
    icmp_packet->icmp_seq = 0;
    memset(icmp_packet->icmp_data, 'A', packet_size - sizeof(struct icmp));
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_cksum = htons(~(htons(icmp_packet->icmp_cksum) + (packet_size << 8)));

    // Send ICMP echo request packet
    num_bytes = sendto(sockfd, buffer, packet_size, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    if (num_bytes < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    printf("Sent ICMP echo request packet to %s\n", inet_ntoa(dest_addr.sin_addr));

    // Wait for ICMP echo reply packet
    num_bytes = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
    if (num_bytes < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    printf("Received ICMP echo reply packet from %s\n", inet_ntoa(dest_addr.sin_addr));

    close(sockfd);
    return 0;
}
