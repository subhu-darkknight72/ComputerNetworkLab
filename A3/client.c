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
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[MAX_SIZE];
	int server_port = atoi(argv[1]);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(server_port);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	receiveStr(buf, sockfd);
	printf("%s\n", buf);
		
	close(sockfd);
	return 0;

}

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