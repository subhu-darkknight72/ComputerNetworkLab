#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <dirent.h>

			/* THE SERVER PROCESS */
const int BUF_SIZE = 51;		//	Packet size for transmission
const int MAX_SIZE = 50001;		//	Max length of strings to be ahndle in operations
char* file="users.txt";			//	User validation file

void sendStr(char* str, int socket_id);
void receiveStr(char *str, int socket_id);
int searchFile(char* filename,char* val);

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[MAX_SIZE];		/* We will use this buffer for communication */

	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed. For internet
	   protocols we use AF_INET. For TCP sockets the second parameter
	   is SOCK_STREAM. The third parameter is set to 0 for user
	   applications.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	/* The structure "sockaddr_in" is defined in <netinet/in.h> for the
	   internet family of protocols. This has three main fields. The
 	   field "sin_family" specifies the family and is therefore AF_INET
	   for the internet family. The field "sin_addr" specifies the
	   internet address of the server. This field is set to INADDR_ANY
	   for machines having a single IP address. The field "sin_port"
	   specifies the port number of the server.
	*/
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	/* This specifies that up to 5 concurrent client
		requests will be queued up while the system is
		executing the "accept" system call below.
	*/
	listen(sockfd, 5);
	
	/* In this program we are illustrating a concurrent server -- one
	   which forks to accept multiple client connections concurrently.
	   As soon as the server accepts a connection from a client, it
	   forks a child which communicates with the client, while the
	   parent becomes free to accept a new connection. To facilitate
	   this, the accept() system call returns a new socket descriptor
	   which can be used by the child. The parent continues with the
	   original socket descriptor.
	*/
	while (1) {
		/* The accept() system call accepts a client connection.
		   It blocks the server until a client request comes.

		   The accept() system call fills up the client's details
		   in a struct sockaddr which is passed as a parameter.
		   The length of the structure is noted in clilen. Note
		   that the new socket descriptor returned by the accept()
		   system call is stored in "newsockfd".
		*/
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					(socklen_t *)&clilen) ;

		/* Having successfully accepted a client connection, the
		   server now forks. The parent closes the new socket
		   descriptor and loops back to accept the next connection.
		*/
		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

		printf("....Socket Connected....\n");
		if (fork() == 0) {
			/* This child process will now communicate with the
			   client through the send() and recv() system calls.
			*/
			
			
			/* Close the old socket since all
			   communications will be through
			   the new socket.
			*/
			close(sockfd);
			
			/* We initialize the buffer, copy the message to it,
			   and send the message to the client. 
			*/
			strcpy(buf,"Message from server");
			sendStr(buf, newsockfd);

			/* We again initialize the buffer, and receive a 
			   message from the client. 
			*/
			receiveStr(buf, newsockfd);
			printf("%s\n", buf);

			//	Asks for Login-id from client
			strcpy(buf,"LOGIN: ");
			sendStr(buf, newsockfd);

			receiveStr(buf, newsockfd);
			printf("%s\n", buf);

			//	Searches for the current user in the validation-file
			int found=searchFile(file,buf);
			printf("user-status: %d\n", found);
			if(found==0){
				//	if not found, informs the client and terminates the socket
				strcpy(buf,"Not-Found");
				sendStr(buf, newsockfd);

				printf("....Socket Closed....\n");
				close(newsockfd);
				exit(0);
			}

			strcpy(buf,"Found");
			sendStr(buf, newsockfd);
			
			//	If user if valid, takes series of commands from the client, and return outputs
			char cmd[5], loc[MAX_SIZE], expr[MAX_SIZE];
			while(1){
				//	Receives the command expression
				receiveStr(expr, newsockfd);
				
				if(strcmp(expr,"exit")==0)	break;
				printf("%s\n",expr);

				memset(cmd, 0, 5);
				if(strlen(expr)==2 || expr[2]==' '){
					strncpy(cmd,expr,2);
					if((strcmp(cmd,"cd")==0) && strlen(expr)<=3){
						
						//	If command = "cd", with no argument
						chdir(getpwuid(getuid())->pw_dir);
						
						if(getcwd(buf, sizeof(buf))==NULL){
		    			    memset(buf, 0, BUF_SIZE);
							strcpy(buf,"####");
							sendStr(buf, newsockfd);
							
							perror("command failed\n");
						}
		    			else{
							// printf("%s\n", buf);
							sendStr(buf, newsockfd);
						}
					}
					else if((strcmp(cmd,"cd")==0)){
						
						//	If command = "cd", with directory-path as argument
						strcpy(loc,expr+3);
						if(chdir(loc) != 0){
							memset(buf, 0, BUF_SIZE);
							strcpy(buf,"####");
							sendStr(buf, newsockfd);

    						perror("command failed\n");
						}
						else{
							if(getcwd(buf, sizeof(buf))==NULL){
		    				    memset(buf, 0, BUF_SIZE);
								strcpy(buf,"####");
								sendStr(buf, newsockfd);

								perror("command failed\n");
							}
		    				else{
								sendStr(buf, newsockfd);
								printf("%s\n", buf);
							}
						}
					}
					else{
						memset(buf, 0, BUF_SIZE);
						strcpy(buf,"$$$$");
						sendStr(buf, newsockfd);
					
						printf("Invalid command!!\n");
						continue;
					}
					// if(getcwd(buf, sizeof(buf))!=NULL)
					// 	printf("%s\n", buf);
				}
				else{
					strncpy(cmd,expr,3);
					if(strcmp(cmd,"pwd")==0){
						
						//	If command = "pwd", return current directory-path
						if(getcwd(buf, sizeof(buf))==NULL){
		    			    memset(buf, 0, BUF_SIZE);
							strcpy(buf,"####");
							sendStr(buf, newsockfd);

							perror("command failed\n");
						}
		    			else{
							sendStr(buf, newsockfd);
							printf("%s\n", buf);
						}
					}
					else if(strcmp(cmd,"dir")==0){
						
						//	If command="dir", get the location to display
						if(strlen(expr)==3)	strcpy(loc,"./");
						else	strcpy(loc, expr+4);

						DIR *d= opendir(loc);
    					struct dirent *dir;
						char file_list[MAX_SIZE];
						strcpy(file_list, "");
    					if(d){
							//	adds the file and folder names to the output string
    					    while ((dir = readdir(d)) != NULL){
    					        // printf("%s\n", dir->d_name);
								strcat(file_list, "   ");
								strcat(file_list, dir->d_name);
								strcat(file_list, "\n");
    					    }
							
							//	send the file & folder list to client
							memset(buf, 0, BUF_SIZE);
							strcpy(buf,file_list);
							sendStr(buf, newsockfd);

    					    closedir(d);
    					}
						else{
							memset(buf, 0, BUF_SIZE);
							strcpy(buf,"####");
							sendStr(buf, newsockfd);

							printf("command failed\n");
						}
					}
					else{
						memset(buf, 0, BUF_SIZE);
						strcpy(buf,"$$$$");
						sendStr(buf, newsockfd);

						printf("Invalid command!!\n");
					}

				}
				
			}

			//	terminate the socket after the user exits
			printf("....Socket Closed....\n");
			close(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
	return 0;
}

/*
	This function searches the user validation file for current user
	
	filename : validation file
	val : current username (string value) we are searching
	
	if user is found, return value = 1.
				Else, return value = 0
*/
int searchFile(char* filename,char* val){
	char    line[50];
    FILE* textfile = fopen(filename, "r");
    if(textfile == NULL)
        return 1;
    
	while(fgets(line, 50, textfile)){
		int l = strlen(line);
		line[l-1]='\0';
		if(strcmp(line,val)==0)
			return 1;
    }
    fclose(textfile);
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