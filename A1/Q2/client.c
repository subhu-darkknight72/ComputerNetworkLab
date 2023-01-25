
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int const BUF_SIZE = 20;
int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	char buf[BUF_SIZE];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server. 

	   In this program, we assume that the server is running on the
	   same machine as the client. 127.0.0.1 is a special address
	   for "localhost" (this machine)
	   
		IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE 
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE 
           MACHINE WHERE YOU ARE RUNNING THE SERVER. 
    	*/

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	for(int i=0; i < BUF_SIZE; i++) buf[i] = '\0';
	recv(sockfd, buf, BUF_SIZE, 0);
	printf("%s\n", buf);

	
	strcpy(buf,"Message from client");
	send(sockfd, buf, strlen(buf) + 1, 0);

	int flag = 0;
	for(int i=0; i<20 && flag==0; i++){
		fgets(buf, BUF_SIZE, stdin);
		send(sockfd, buf, strlen(buf) + 1, 0);
		printf("%ld, sent.\n",strlen(buf));

		if(strlen(buf)<(BUF_SIZE-1))	flag=1;
		for(int j=0; j<strlen(buf) && flag==0;j++)
			if(buf[j]=='\n')
				flag=1;
	}

	strcpy(buf,"end");
	printf("%s\n",buf);
	send(sockfd, buf, strlen(buf) + 1, 0);
	
	close(sockfd);
	return 0;

}
