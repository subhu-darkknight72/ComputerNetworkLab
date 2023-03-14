#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// include header for thread
#include <pthread.h>

const int BUF_SIZE = 5;
const int MAX_LEN = 1024;
typedef struct mssg_table{
    int write_ptr;
    int read_ptr;
    int size;
    char **table;
} mssg_table;

mssg_table* mssg_table_init(int size){
    mssg_table *mt = (mssg_table *)calloc(1,sizeof(mssg_table));
    mt->write_ptr = 0;
    mt->read_ptr = 0;
    mt->size = 0;
    mt->table = (char **)calloc(BUF_SIZE, sizeof(char *));
    return mt;
}

void mssg_table_read(mssg_table *mt, char *mssg){
    // while(mt->size == 0);// wait till smt->size > 0
    // printf("in mssg_table_read\n");
    int pos = mt->read_ptr;
    // printf("pos = %d\n",pos);
    strcpy(mssg, mt->table[pos]);
    printf("READ mt->table[%d] = %s",pos,mt->table[pos]);

    mt->read_ptr = (pos+1) % BUF_SIZE;
    mt->size = mt->size - 1;
}

void mssg_table_write(mssg_table *mt, char *mssg){
    // while(mt->size == BUF_SIZE);// wait till smt->size < BUF_SIZE

    printf("in mssg_table_write\n");
    int pos = mt->write_ptr;

    mt->table[pos] = (char *)calloc(MAX_LEN,sizeof(char));
    strcpy(mt->table[pos], mssg);

    printf("WRTIE smt->table[%d] = %s",pos, mt->table[pos]);
    
    mt->write_ptr = (pos+1) % BUF_SIZE;
    mt->size = mt->size + 1;
}

void print_mssg_table(mssg_table *mt){
    printf("write_ptr = %d, read_ptr = %d, size = %d\n",mt->write_ptr,mt->read_ptr,mt->size);
    for(int i = 0; i < BUF_SIZE; i++){
        printf("mt->table[%d] = %s\n",i,mt->table[i]);
    }
    printf("-----------------------------------------------------\n");
}

int main(){
    mssg_table *smt = mssg_table_init(1024);
    char *mssg = (char *)calloc(1024,sizeof(char));
    
    // strcpy(mssg,"Hii Geetu!!");
    // printf("mssg = %s\n",mssg);
    // mssg_table_write(smt,mssg);

    // print_mssg_table(smt);

    // mssg_table_read(smt,mssg);
    // printf("mssg = %s\n",mssg);

    // print_mssg_table(smt);

    int cmd = 1;
    while(cmd != 0){
        printf("Enter 0 to exit, 1 to read, 2 to write: ");
        scanf("%d",&cmd);
        if(cmd == 1){
            //  read message from smt
            mssg_table_read(smt,mssg);
            printf("mssg = %s\n",mssg);
        }
        else if(cmd == 2){
            // write message to smt from stdin
            scanf("%s",mssg);
            printf("mssg = %s\n",mssg);
            mssg_table_write(smt,mssg);
        }
        print_mssg_table(smt);
    }
    return 0;
}