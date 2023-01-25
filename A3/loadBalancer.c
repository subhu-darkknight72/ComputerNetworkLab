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
	int	sockfd_1, sockfd_2; /* Socket descriptors */
	struct sockaddr_in serv1_addr, serv2_addr;

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

	serv1_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv1_addr.sin_port		= htons(s1_port);

	serv2_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv2_addr.sin_port		= htons(s2_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	struct pollfd S[2];
    S[0].fd = sockfd;		S[0].events = POLLIN;
	S[1].fd = sockfd;		S[1].events = POLLIN;

    int timeout = 5000;
    
    int n;
    socklen_t len; 

	while(1){
        int p_val = poll(S,1,timeout);
        char *hello = "...CLIENT connected..."; 
        sendto(sockfd, (const char *)hello, strlen(hello), 0, 
	    		(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        // printf("Hello message sent from client\n"); 

        if(p_val<0){
            perror("Poll-Error Message !!\n");
            exit(0);
        }
        else if(p_val>0){
            
            //  successfully connected to server
            len = sizeof(servaddr);
            char buffer[MAXLINE]; 

            //  retrieve the server time
            n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
	        		( struct sockaddr *) &servaddr, &len); 
            buffer[n] = '\0'; 
            printf("%s\n", buffer); 
            break;
        }
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