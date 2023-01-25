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
	int	sockfd_1, sockfd_2, sockfd, newsockfd; /* Socket descriptors */
	struct sockaddr_in cli_addr, serv1_addr, serv2_addr;

	int s1_port = atoi(argv[1]);
	int s2_port = atoi(argv[2]);
	int c0_port = atoi(argv[3]);

	char buf[MAX_SIZE];		/* We will use this buffer for communication */
	if ((sockfd_1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket-1\n");
		exit(0);
	}
	if ((sockfd_2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket-2\n");
		exit(0);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv1_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv1_addr.sin_addr);
	serv1_addr.sin_port		= htons(s1_port);

	serv2_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv2_addr.sin_addr);
	serv2_addr.sin_port		= htons(s2_port);

	if ((connect(sockfd_1, (struct sockaddr *) &serv1_addr,
						sizeof(serv1_addr))) < 0) {
		perror("Unable to connect to server-1\n");
		exit(0);
	}
	else
		printf("Connected to server-1 successfully!!\n");
	
	if ((connect(sockfd_2, (struct sockaddr *) &serv2_addr,
						sizeof(serv2_addr))) < 0) {
		perror("Unable to connect to server-2\n");
		exit(0);
	}
	else
		printf("Connected to server-2 successfully!!\n");

	receiveStr(buf, sockfd_1);
	printf("Server1: %s\n",buf);

	receiveStr(buf, sockfd_2);
	printf("Server2: %s\n",buf);

	strcpy(buf,"Message from client to Server-1");
	sendStr(buf, sockfd_1);

	strcpy(buf,"Message from client to Server-2");
	sendStr(buf, sockfd_2);
	
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