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

			/* THE SERVER PROCESS */

int const BUF_SIZE = 100;
double calc(char* str,int l);
int opr(char ch);

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	char buf[BUF_SIZE];		/* We will use this buffer for communication */

	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed. For internet
	   protocols we use AF_INET. For TCP sockets the second parameter
	   is SOCK_STREAM. The third parameter is set to 0 for user
	   applications.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
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
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); /* This specifies that up to 5 concurrent client
			      requests will be queued up while the system is
			      executing the "accept" system call below.
			   */

	/* In this program we are illustrating an iterative server -- one
	   which handles client connections one by one.i.e., no concurrency.
	   The accept() system call returns a new socket descriptor
	   which is used for communication with the server. After the
	   communication is over, the process comes back to wait again on
	   the original socket descriptor.
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
					( socklen_t *)&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}


		/* We initialize the buffer, copy the message to it,
			and send the message to the client. 
		*/
		
		strcpy(buf,"Message from server");
		send(newsockfd, buf, strlen(buf) + 1, 0);

		/* We now receive a message from the client. For this example
  		   we make an assumption that the entire message sent from the
  		   client will come together. In general, this need not be true
		   for TCP sockets (unlike UDPi sockets), and this program may not
		   always work (for this example, the chance is very low as the 
		   message is very short. But in general, there has to be some
	   	   mechanism for the receiving side to know when the entire message
		  is received. Look up the return value of recv() to see how you
		  can do this.
		*/ 
		recv(newsockfd, buf, BUF_SIZE, 0);
		printf("%s\n", buf);

		char expr[]="0+";
		printf("Loop starting...\n");
		for(int i=0; 1; i++){
			printf("%d: ",i);
			
			for(int j=0; j < BUF_SIZE; j++) buf[j] = '\0';
			recv(newsockfd, buf, BUF_SIZE, 0);
			printf("%s\n",buf);
			if(strcmp(buf, "end")==0)	break;
			strcat(expr, buf);
		}
		printf("%ld: %s\n",strlen(expr), expr);

		double ans = calc(expr, strlen(expr));
		printf("Ans: %f\n",ans);
		close(newsockfd);
	}
	return 0;
}
			
int opr(char ch){
    if(ch=='+') return 1;
    if(ch=='-') return 2;
    if(ch=='*') return 3;
    if(ch=='/') return 4;
    if(ch=='(' || ch==')')  return 5;
	if((ch>='0' && ch<='9') || ch=='.')	return 0;
    return -1;
}

double calc(char* str,int l){
    char x0[32], x[32], op='+';
    char *ptr;

    int pos=0;
    while(pos<l && opr(str[pos])==0) pos++;
    
    double ans, val;
    if(pos>0){
        strncpy(x0, str, pos);
        ans = strtod(x0, &ptr);
    }
    else    ans = 0;

    while(pos<l){
		if(opr(str[pos])<0){
			pos++;
			continue;
		}
        if(pos>0)   op=str[pos++];

        if(str[pos]=='('){
            int e = pos+1;
            for(;e<l && str[e]!=')';e++);
            strncpy(x, str+pos+1, e-pos-1);
            val = calc(x, e-pos+1);
            pos = e+1;
        }
        else{
            int e=pos;
            while(e<l && opr(str[e])==0) e++;
            strncpy(x, str+pos, e-pos);
            pos=e;

            val = strtod(x, &ptr);
        }
        // printf("%f %c %f \n",ans, op, val);

        if(op=='+')         ans=ans+val;
        else if(op=='-')    ans=ans-val;
        else if(op=='*')    ans=ans*val;
        else if(op=='/')    ans=ans/val;
    }
    return ans;
}