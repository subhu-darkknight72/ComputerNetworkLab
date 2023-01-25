#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Min(a,b) (a<b?a:b)
const int BUF_SIZE = 51;		//	Packet size for transmission
const int MAX_SIZE = 50001;		//	Max length of strings to be handled in operations

void sendStr(char* str, int socket_id);
void receiveStr(char *str, int socket_id);

int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char cmd[MAX_SIZE];

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
	else{
		printf("Connected to server successfully!!\n");
	}

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	receiveStr(cmd, sockfd);
	printf("%s\n", cmd);

	strcpy(cmd,"Message from client");
	sendStr(cmd, sockfd);

	//	Server is aksing for user-id
	receiveStr(cmd, sockfd);
	printf("%s", cmd);

	//	getting the username from stdin stream
	fgets(cmd, MAX_SIZE, stdin);
	int l1 = strlen(cmd);
    cmd[l1-1]='\0';

	//	send username to server
	sendStr(cmd, sockfd);
	//	get user-status from server
	receiveStr(cmd, sockfd);
	
	//	If user not-found, exit. Else log-in user
	if(strcmp(cmd,"Not-Found")==0){
		printf("User Not-Found!!\n");
		close(sockfd);
		return 0;
	}
	printf("...User Logged-in...\n");
	
	char temp[5];
	while(1){
		//	accepting command-inputs from user.
		printf("subhu ~%% ");
		memset(cmd, 0, MAX_SIZE);
        fgets(cmd, MAX_SIZE, stdin);
		if(strcmp(cmd,"\n")==0)	continue;

		int l0 = strlen(cmd);
        cmd[l0-1]='\0';
				
		strcpy(cmd,cmd);
		sendStr(cmd, sockfd);

		int flag=0;
		if(strcmp(cmd,"exit")==0)	break;
		if(strlen(cmd)>=2){
			strncpy(temp,cmd,2);
			// printf("%s\n",temp);
			// printf("%d\n",strcmp(temp,"cd"));
			if(strcmp(temp,"cd")==127){
				flag=1;
				// printf("flag changed!!\n");
			}
		}

		/*
			get the command output from the user
			
			pwd	   : print the current directory path
			cd	   : change directory to "HOME"
			cd $1  : change directory to "$1"
			dir	   : print the list of files & folders in current directory
			dir $1 : print the list of files & folders in "$1"
		*/
		for(i=0; i < MAX_SIZE; i++) cmd[i] = '\0';
		receiveStr(cmd, sockfd);
		if(strcmp(cmd,"$$$$")==0)
			printf("Invalid command\n");
		else if(strcmp(cmd,"####")==0)
			printf("Error in running command\n");
		else if(flag==0)
			printf("%s\n",cmd);

		//	flush stdin, before taking next command input
		fflush(stdin);
	};

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