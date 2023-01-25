#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

			/* THE SERVER PROCESS */
const int BUF_SIZE = 20;
long double calc(char* str,int l);
int opr(char ch);

int main(){
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

			/* We will use this buffer for communication */

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);
	while (1) {
		char buf[BUF_SIZE];

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					(socklen_t *)&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}
		
		strcpy(buf,"Message from server");
		send(newsockfd, buf, strlen(buf) + 1, 0);

		recv(newsockfd, buf, BUF_SIZE, 0);
		printf("%s\n", buf);

		char expr[]="0+";
		for(int i=1;i<20; i++){
			recv(newsockfd, buf, BUF_SIZE, 0);
			// printf("%d: %ld\n",i,strlen(buf));
			// printf("%d (%ld): %s\n", i,strlen(buf), buf);

			if(strcmp("end",buf)==0)
				break;
			strcat(expr, buf);
		}
		printf("String Recieved successfully...\n");
		// printf("%ld: %s\n",strlen(expr), expr);
		
		double ans = calc(expr,strlen(expr));
		// printf("Ans: %f\n",ans);

		sprintf(buf, "%f", ans);
		// printf("Ans: %s\n",buf);
		send(newsockfd, buf, strlen(buf) + 1, 0);
		printf("Answer sent back successfully...\n");

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

long double calc(char* str,int l){
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
        // printf("%Lf %c %Lf \n",ans, op, val);

        if(op=='+')         ans=ans+val;
        else if(op=='-')    ans=ans-val;
        else if(op=='*')    ans=ans*val;
        else if(op=='/')    ans=ans/val;
    }
    return ans;
}
