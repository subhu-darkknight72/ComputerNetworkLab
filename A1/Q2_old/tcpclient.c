#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int BUF_SIZE = 20;
int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];

	/* Opening a socket is exactly similar to the server process */
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

	for(i=0; i < BUF_SIZE; i++) buf[i] = '\0';
	recv(sockfd, buf, BUF_SIZE, 0);
	printf("%s\n", buf);

	
	strcpy(buf,"Message from client");
	send(sockfd, buf, strlen(buf) + 1, 0);

	printf("Enter Expression:\n");
	int flag=0;
	for(int i=1;i<10000 && flag==0; i++){
		fgets(buf, BUF_SIZE, stdin);
		// printf("%d: %s\n", i, buf);
		send(sockfd, buf, strlen(buf) + 1, 0);
		
		if(strlen(buf)<(BUF_SIZE-1))	flag=1;
		for(int j=0; j<BUF_SIZE && flag==0; j++)
			if(buf[j]=='\n')
				flag=1;
	}
	char t[]="end";
	strcpy(buf,t);
	send(sockfd, buf, strlen(buf) + 1, 0);
	// printf("%s \n",buf);
	
	for(i=0; i < BUF_SIZE; i++) buf[i] = '\0';
	recv(sockfd, buf, BUF_SIZE, 0);
	printf("Answer: %s\n", buf);

	close(sockfd);
	return 0;

}

