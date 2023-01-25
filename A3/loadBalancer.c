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
	int	clilen;
	int	sockfd, newsockfd; /* Socket descriptors */
	int sockfd_s[2];
	struct sockaddr_in cli_addr, serv_addr, serv1_addr, serv2_addr;

	int s1_port = atoi(argv[1]);
	int s2_port = atoi(argv[2]);
	int c0_port = atoi(argv[3]);

	char buf[MAX_SIZE];		/* We will use this buffer for communication */
	if ((sockfd_s[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create server-socket-1\n");
		exit(0);
	}
	if ((sockfd_s[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create server-socket-2\n");
		exit(0);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create client-socket\n");
		exit(0);
	}

	serv1_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv1_addr.sin_addr);
	serv1_addr.sin_port		= htons(s1_port);

	serv2_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv2_addr.sin_addr);
	serv2_addr.sin_port		= htons(s2_port);

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(c0_port);

	if ((connect(sockfd_s[0], (struct sockaddr *) &serv1_addr, sizeof(serv1_addr))) < 0) {
		perror("Unable to connect to server-1\n");
		exit(0);
	}
	else
		printf("Connected to server-1 successfully!!\n");
	
	if ((connect(sockfd_s[1], (struct sockaddr *) &serv2_addr, sizeof(serv2_addr))) < 0) {
		perror("Unable to connect to server-2\n");
		exit(0);
	}
	else
		printf("Connected to server-2 successfully!!\n");

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	else
		printf("Binded client-side address successfully!!\n");

	listen(sockfd, 5);
	while(1){
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					(socklen_t *)&clilen) ;

		int l0, l1, k=0;

		receiveStr(buf, sockfd_s[0]);
		l0 = atoi(buf);

		receiveStr(buf, sockfd_s[1]);
		l1 = atoi(buf);

		printf("Server1 Load: %d\n",l0);
		printf("Server2 Load: %d\n",l1);

		if(l1<l0)	k=1;
		
		strcpy(buf,"kill");
		sendStr(buf, sockfd_s[1-k]);

		strcpy(buf,"time");
		sendStr(buf, sockfd_s[k]);
		receiveStr(buf, sockfd_s[k]);

		sendStr(buf, newsockfd);
		receiveStr(buf, newsockfd);
		printf("%s\n", buf);

		strcpy(buf,"Message from client to Server-1");
		sendStr(buf, sockfd_s[0]);

		strcpy(buf,"Message from client to Server-2");
		sendStr(buf, sockfd_s[1]);
		close(newsockfd);
	}
	close(sockfd_s[0]);
	close(sockfd_s[1]);
	close(sockfd);
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