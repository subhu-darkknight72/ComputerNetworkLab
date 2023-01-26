#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>

/* THE SERVER PROCESS */
const int BUF_SIZE = 1001; //	Packet size for transmission
const int MAX_SIZE = 1001; //	Max length of strings to be ahndle in operations
void sendStr(char *str, int socket_id);
void receiveStr(char *str, int socket_id);

int main(int argc, char *argv[])
{
	int sockfd, newsockfd; /* Socket descriptors */
	int clilen;
	struct sockaddr_in cli_addr, serv_addr;
	srand(time(0));

	int i;
	char buf[MAX_SIZE]; /* We will use this buffer for communication */
	int server_port = atoi(argv[1]);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(server_port);

	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
	{
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);

	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);

		if (newsockfd < 0)
		{
			perror("Accept error\n");
			exit(0);
		}

		receiveStr(buf, newsockfd);
		printf("Command: %s\n",buf);
		if (strcmp(buf, "load") == 0)
		{
			int val = (rand()) % 100;
			snprintf(buf, MAX_SIZE, "%d", val);
			printf("Load: %s\n", buf);
			sendStr(buf, newsockfd);
		}
		else if (strcmp(buf, "time") == 0)
		{
			time_t mytime = time(NULL);
			char *time_str = ctime(&mytime);
			time_str[strlen(time_str) - 1] = '\0';
			sendStr(time_str, newsockfd);
			printf("Time sent!!\n");
		}

		close(newsockfd);
	}
	
	close(sockfd);
	return 0;
}

void sendStr(char *str, int socket_id)
{
	int pos, i, len = strlen(str);
	char buf[BUF_SIZE];

	for (pos = 0; pos < len; pos += BUF_SIZE)
	{
		for (i = 0; i < BUF_SIZE; i++)
			buf[i] = ((pos + i) < len) ? str[pos + i] : '\0';

		// send(socket_id, buf, BUF_SIZE, 0);
		if (send(socket_id, buf, BUF_SIZE, 0) < 0)
		{
			perror("error in transmission.\n");
			exit(-1);
		}
	}
}

void receiveStr(char *str, int socket_id)
{
	int flag = 0, i, pos = 0;
	char buf[BUF_SIZE];
	memset(str, 0, MAX_SIZE);
	while (flag == 0)
	{
		// recv(socket_id, buf, BUF_SIZE, 0)
		memset(buf, 0, BUF_SIZE);
		if (recv(socket_id, buf, BUF_SIZE, 0) < 0)
		{
			perror("error in transmission.\n");
			exit(-1);
		}
		// printf("$%s$\n",buf);

		for (i = 0; i < BUF_SIZE && flag == 0; i++)
			if (buf[i] == '\0')
				flag = 1;

		for (i = 0; i < BUF_SIZE; i++, pos++)
			str[pos] = buf[i];
	}
}