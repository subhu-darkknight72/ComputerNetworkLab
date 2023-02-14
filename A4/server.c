/**httpserver.c**/
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "string.h"
#include "netinet/in.h"
#include "time.h"
#include "dirent.h"
#include "netdb.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_SIZE 1000000
#define BUF_SIZE 1024
#define CLADDR_LEN 10

char buffer[BUF_SIZE];

int createSocket(char *host, int port);
int listenForRequest(int sockfd);
char *getFileType(char *file);
int parseHeader(char *header);
char *splitKeyValue(char *line, int index);
void openFile();
void get_req();
void put_req();

void sendStr(char *str, int socket_id);
void receiveStr(char *str, int socket_id);

char http_not_found[] = "HTTP/1.0 404 Not Found\n";
char http_ok[] = "HTTP/1.0 200 OK\n";

char filepath[BUF_SIZE];
char *contentType;
char status[4] = {0, 0, 0, 0};
char keys[][25] = {"Date: ", "Hostname: ", "Location: ", "Content-Type: "};

DIR *dirptr;
FILE *fileptr;
time_t timenow;

char *header, *request, *path, *newpath, *host, *hst, *body, *ip;
char *dir, *temp, filename[MAX_SIZE];
int port, sockfd, newsockfd;
char *get, *http;

int main(int argc, char **argv)
{
    host = (char *)calloc(10000, sizeof(char));
    dir = (char *)calloc(10000, sizeof(char));
    strcpy(host, "127.0.0.1");
    strcpy(dir, "/Users/subhu/Desktop/Sem/Sem_6/CN_Lab/ComputerNetworkLab/A4");
    port = atoi("8080");

    if ((dirptr = opendir(dir)) == NULL)
    {
        printf("Directory Not Found!\n");
        exit(1);
    }

    sockfd = createSocket(host, port);
    if (listen(sockfd, 5) < 0)
    {
        printf("Error listening to socket!\n");
        exit(1);
    }

    while (1)
    {
        printf("--------------------------------------------------------\n");
        printf("Waiting for a connection...\n");
        newsockfd = listenForRequest(sockfd);

        request = (char *)malloc(MAX_SIZE * sizeof(char));
        newpath = (char *)malloc(MAX_SIZE * sizeof(char));
        header = (char *)calloc(MAX_SIZE, sizeof(char));
        receiveStr(header, newsockfd);
        printf("Header: \n--------\n%s\n--------\n", header);

        get = (char *)calloc(10, sizeof(char));
        path = (char *)calloc(MAX_SIZE, sizeof(char));
        http = (char *)calloc(MAX_SIZE, sizeof(char));
        hst = (char *)malloc(MAX_SIZE * sizeof(char));
        ip = (char *)malloc(MAX_SIZE * sizeof(char));
        body = (char *)malloc(MAX_SIZE * sizeof(char));

        if (strncmp(header, "GET", 3) == 0)
        {
            printf("GET request received!\n");
            get_req();
        }
        else
        {
            put_req();
        }
    }

    close(sockfd);
    free(header);
    free(request);
    free(path);
    free(newpath);
    free(hst);
    free(ip);
    free(body);

    return 0;
}

void get_req()
{
    printf("$%s$\n", request);

    printf("Processing request...\n");

    // parses request
    sscanf(request, "%s %s %s", get, path, http);
    // printf("newsockfd2:%d\n",newsockfd); // file descriptor is changing after sscanf command

    newpath = path + 1; // ignores the first slash
    sprintf(filepath, "%s/%s", dir, newpath);

    printf("filepath= $%s$\n", filepath);
    printf("dir= $%s$\n", dir);
    printf("newpath= $%s$\n", newpath);

    contentType = getFileType(newpath);

    timenow = time(NULL);
    struct tm *timeinfo = localtime(&timenow);
    sprintf(header, "HTTP/1.0 200 OK\nDate: %sHostname: %s:%d\nLocation: %s\nContent-Type: %s\n\n", asctime(timeinfo), host, port, newpath, contentType);

    if ((fileptr = fopen(filepath, "r")) == NULL)
    {
        printf("File not found!\n");
        send(newsockfd, http_not_found, strlen(http_not_found), 0); // sends HTTP 404
    }
    else
    {
        printf("Sending the file...\n");

        // http_ok = (char *)calloc(10000, sizeof(char));
        // strcpy(http_ok, "HTTP/1.0 200 OK");
        printf("$%s$\n", http_ok);
        // printf("newsockfd:%d\n",newsockfd);  // file descriptor is changing
        ssize_t bytes_sent = send(newsockfd, http_ok, strlen(http_ok) + 1, 0); // sends HTTP 200 OK
        printf("bytes_sent: %zd, length of http_ok: %lu\n", bytes_sent, strlen(http_ok));

        memset(buffer, 0, BUF_SIZE);
        recv(newsockfd, buffer, BUF_SIZE, 0);
        printf("$%s$\n", buffer);
        if ((temp = strstr(buffer, "OK")) == NULL)
        {
            printf("Operation aborted by the user!\n");
            return;
        }
        send(newsockfd, header, strlen(header), 0); // sends the header
        recv(newsockfd, buffer, BUF_SIZE, 0);
        if ((temp = strstr(buffer, "OK")) == NULL)
        {
            printf("Operation aborted by the user!\n");
            return;
        }
        memset(&buffer, 0, sizeof(buffer));
        while (!feof(fileptr))
        { // sends the file
            fread(&buffer, sizeof(buffer), 1, fileptr);
            send(newsockfd, buffer, sizeof(buffer), 0);
            memset(&buffer, 0, sizeof(buffer));
        }
        printf("File sent...\n");
    }
    printf("Processing completed...\n");
    close(newsockfd);
}

void put_req()
{
    sscanf(request, "%s %s %s\n%s %s\n\n%s", get, path, http, hst, ip, body);

    newpath = path + 1; // ignores the first slash
    sprintf(filepath, "%s/%s", dir, newpath);
    printf("get=$%s$\n", get);
    printf("filepath= $%s$\n", filepath);
    printf("http=&%s&", http);
    printf("dir= $%s$\n", dir);
    printf("newpath= $%s$\n", newpath);
    printf("body=%s\n", body);

    strcpy(contentType, getFileType(newpath));
    printf("Content-Type: %s\n", contentType);

    struct tm *timeinfo;
    time(&timenow);
    timeinfo = localtime(&timenow);
    sprintf(header, "Date: %sHostname: %s:%d\nLocation: %s\nContent-Type: %s\n\n", asctime(timeinfo), host, port, newpath, contentType);

    if ((fileptr = fopen(filepath, "w")) == NULL)
    {
        printf("File not found!\n");
        send(newsockfd, http_not_found, strlen(http_not_found), 0); // sends HTTP 404
        close(newsockfd);
        return;
    }

    printf("File found!\n");
    printf("$ %s $\n", http_ok);

    printf("Processing completed...\n");
    close(newsockfd);
    openFile();
}

int createSocket(char *host, int port)
{
    int sockfd;
    struct sockaddr_in addr;
    struct hostent *host_ent;
    char *hostAddr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((short)port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Error creating socket!\n");
        exit(1);
    }
    printf("Socket created...\n");

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Error binding socket to port!\n");
        exit(1);
    }
    printf("Binding done...\n");

    return sockfd;
}

int listenForRequest(int sockfd)
{
    int conn;
    char hostip[32];
    struct sockaddr_in addr;
    struct hostent *host;
    struct in_addr inAddr;
    int len;

    addr.sin_family = AF_INET;

    listen(sockfd, 5); // maximum 5 connections
    len = sizeof(addr);
    if ((conn = accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)&len)) < 0)
    {
        printf("Error accepting connection!\n");
        exit(1);
    }
    printf("Connection accepted...\n");

    inet_ntop(AF_INET, &(addr.sin_addr), hostip, 32);
    inet_pton(AF_INET, hostip, &inAddr);
    host = gethostbyaddr(&inAddr, sizeof(inAddr), AF_INET);

    printf("---Connection received from: %s [IP= %s]---\n", host->h_name, hostip);
    return conn;
}

char *getFileType(char *file)
{
    char *temp;
    if ((temp = strstr(file, ".html")) != NULL)
    {
        return "text/html";
    }
    else if ((temp = strstr(file, ".pdf")) != NULL)
    {
        return "application/pdf";
    }
    else if ((temp = strstr(file, ".txt")) != NULL)
    {
        return "text/html";
    }
    else if (((temp = strstr(file, ".jpeg")) != NULL) || ((temp = strstr(file, ".heic")) != NULL))
    {
        return "image/jpeg";
    }
    return "text/*";
}

char *splitKeyValue(char *line, int index)
{
    char *temp;
    if ((temp = strstr(line, keys[index])) != NULL)
    {
        temp = temp + strlen(keys[index]);
        status[index] = 1;
    }
    return temp;
}

int parseHeader(char *header)
{
    char *line, *key, *value;
    char temp[100];
    int i = 0;
    line = strtok(header, "\n");
    while (line != NULL)
    {
        // printf("%s\n", line);
        strcpy(temp, line);
        value = splitKeyValue(line, i);
        if (i == 3)
        {
            strcpy(contentType, value);
        }
        // printf("value=%s\n", value);
        line = strtok(NULL, "\n");
        i++;
    }
    for (i = 0; i < 4; i++)
    {
        if (status[i] == 0)
            return 1;
        // printf("status[%d]=%d\n", i, status[i]);
    }
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

void openFile()
{
    char *temp;
    char command[100];
    char fileName[1000];
    strcpy(fileName, filepath);
    // printf("File Name: %s\n", fileName);
    // printf("Content Type: %s\n", contentFileType);
    if ((temp = strstr(contentType, "text/html")) != NULL)
    {
        if ((temp = strstr(fileName, ".txt")) != NULL)
        {
            sprintf(command, "open -a TextEdit %s", fileName);
        }
        else
        {
            sprintf(command, "firefox %s", fileName);
        }
        system(command);
    }
    else if ((temp = strstr(contentType, "application/pdf")) != NULL)
    {
        sprintf(command, "open -a Preview.app %s", fileName);
        system(command);
    }
    else if ((temp = strstr(contentType, "image/jpeg")) != NULL)
    {
        sprintf(command, "open -a Preview.app %s", fileName);
        system(command);
    }
    else
    {
        printf("The filetype %s is not supported. Failed to open %s!\n", contentType, fileName);
    }
}
