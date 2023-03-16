
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[1000];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}


	for(i=0; i < 1000; i++) buf[i] = '\0';
	int n = recv(sockfd, buf, 1000, 0);
	printf("bytes received=%d, %s\n", n, buf);

	for(i=0; i < 1000; i++) buf[i] = '\0';
	n = recv(sockfd, buf, 1000, 0);
	printf("bytes received=%d, %s\n", n, buf);

	memset(buf, 0, 1000);
	strcpy(buf,"Message from client 1");
	int l = strlen(buf)+1;
	send(sockfd, &l, sizeof(l), 0);
	send(sockfd, buf, l, 0);

	memset(buf, 0, 1000);
	strcpy(buf,"Message from client 2");
	l = strlen(buf)+1;
	send(sockfd, &l, sizeof(l), 0);
	send(sockfd, buf, l, 0);
		
	close(sockfd);
	return 0;

}

