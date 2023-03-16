
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
	recv(sockfd, buf, 1000, 0);
	printf("%s\n", buf);

	
	strcpy(buf,"Message from client 1");
	send(sockfd, buf, strlen(buf) + 1, 0);

	for(i=0; i < 1000; i++) buf[i] = '\0';
	recv(sockfd, buf, 1000, 0);
	printf("%s\n", buf);

	
	strcpy(buf,"Message from client 2");
	send(sockfd, buf, strlen(buf) + 1, 0);
		
	close(sockfd);
	return 0;

}

