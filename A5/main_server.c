#include <stdio.h>
#include "mysocket.h"

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];		/* We will use this buffer for communication */

	if ((sockfd = my_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (my_bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}
	my_listen(sockfd, 5);
	
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = my_accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}
		
		memset(buf, 0, 100);
		strcpy(buf, "Message from server");
		send(newsockfd, buf, strlen(buf) + 1, 0);

		recv(newsockfd, buf, 100, 0);
		printf("%s\n", buf);

		my_close(newsockfd);
	}
	return 0;
}