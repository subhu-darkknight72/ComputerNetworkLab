/*
			NETWORK PROGRAMMING WITH SOCKETS

In this program we illustrate the use of Berkeley sockets for interprocess
communication across the network. We show the communication between a server
process and a client process.


*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

const int BUF_SIZE = 1001;		//	Packet size for transmission
const int MAX_SIZE = 1001;		//	Max length of strings to be ahndle in operations
void sendStr(char* str, int socket_id);
void receiveStr(char *str, int socket_id);

int main(int argc,char* argv[])
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;


	
	int s1_port = atoi(argv[1]);
	int s2_port = atoi(argv[2]);

	int i;
	char buf[MAX_SIZE];		/* We will use this buffer for communication */

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

		if (fork() == 0) {
			close(sockfd);
			strcpy(buf,"Message from server");
			sendStr(buf, newsockfd)

			receiveStr(buf, newsockfd)
			printf("%s\n", buf);

			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
	return 0;
}

/*
	takes a string "str" (may be long), and send it to the client
	by splitting the string into small chunks.
*/
void sendStr(char* str, int socket_id){
    int pos, i, len=strlen(str);
    char buf[BUF_SIZE];

    for(pos=0; pos<len; pos+=BUF_SIZE){
        for(i=0; i<BUF_SIZE; i++)
            buf[i] = ((pos+i)<len) ? str[pos+i]: '\0';

		// send(socket_id, buf, BUF_SIZE, 0);
        if(send(socket_id, buf, BUF_SIZE, 0) < 0){
            perror("error in transmission.\n");
            exit(-1);
        }
    }
}

/*
	receives string "str" (may be long) from the client
	by concatenating the incoming string chunk stream
*/
void receiveStr(char *str, int socket_id){
    int flag=0, i, pos=0;
    char buf[BUF_SIZE];
    while(flag==0){
        // recv(socket_id, buf, BUF_SIZE, 0)
		if( recv(socket_id, buf, BUF_SIZE, 0) < 0 ){
            perror("error in transmission.\n");
            exit(-1);
        }
		// printf("$%s$\n",buf);

        for(i=0; i<BUF_SIZE && flag==0; i++)
            if(buf[i]=='\0') 
				flag=1;

        for(i=0; i<BUF_SIZE; i++, pos++)	
			str[pos] = buf[i];
    }
}