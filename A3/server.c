#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>


			/* THE SERVER PROCESS */

int main(int argc,char* argv[])
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];		/* We will use this buffer for communication */
	int server_port = atoi(argv[1]);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(server_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					(socklen_t*) &clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		time_t mytime = time(NULL);
    	char * time_str = ctime(&mytime);
    	time_str[strlen(time_str)-1] = '\0';

		send(newsockfd, time_str, strlen(time_str) + 1, 0);

		recv(newsockfd, buf, 100, 0);
		printf("%s\n", buf);

		close(newsockfd);
	}
	return 0;
}
			

