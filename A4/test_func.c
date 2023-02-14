#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char *port_n;
char *get_filename(char *url_var){
    char *ptr, *host_var, *ans;
    char pth[1024];
    if ((ptr = strstr(url_var, "/")) == NULL)
    {
        // when hostname does not contain a slash
        printf("no slash");
        return url_var;
    }
    else
    {
        // when hostname contains a slash, it is a pth to file
        char *fname;
        fname = (char *)calloc(1024, sizeof(char));
        ans = (char *)calloc(1024, sizeof(char));
        strcpy(pth, ptr);
        host_var = strtok(url_var, "/");
        while (host_var != NULL)
        {   
            strcpy(fname, host_var);
            // printf("host_var: %s\n", host_var);
            host_var = strtok(NULL, "/");
        }
        fname = strtok(fname, ":");
        strcpy(ans, fname);
        fname = strtok(NULL, ":");
        if(fname!=NULL){
            strcpy(port_n, fname);
        }
        // printf("pth: %s", fname);
        return ans;
    }
}


char **get_arg(char *cmd){
    char cmd_copy[1000] = {0};
    strcpy(cmd_copy, cmd);
    char **arg;

    char *p = strtok(cmd, " ");
    int cnt = 0;
    while (p){
        p = strtok(NULL, " ");
        cnt++;
    }
    arg = (char **)calloc((cnt + 1), sizeof(char *));
    p = strtok(cmd_copy, " ");
    int i = 0;
    for (; p; i++)
    {
        arg[i] = (char *)calloc((strlen(p) + 1), sizeof(char));
        strcpy(arg[i], p);
        arg[i][strlen(p)] = '\0';
        p = strtok(NULL, " ");
    }

    arg[i] = NULL;
    return arg;
}

int runExtCmd0(char *usr_cmd){
    char **arg = get_arg(usr_cmd);
    execvp(arg[0], arg);
    return 0;
}

int main()
{
    printf("Hello, World!\n");
    // get the file name from given url_var
    
    // char url_var[] = "http://10.98.78.2/docs/a1.pdf:8081";
    // // char url_var[] = "http://cse.iitkgp.ac.in/~agupta/networks/index.html";
    // port_n = (char *)calloc(100, sizeof(char));
    // strcpy(port_n, "8080");
    // char *f_name = get_filename(url_var);
    // printf("filename: %s\n", f_name);
    // printf("port: %s\n", port_n);

    char cmd[1000];

    char *file="tmpfile.csv";
    strcpy(cmd, "lsof ./");
    runExtCmd0(cmd);
    return 0;
}