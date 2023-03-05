#include <stdio.h>
#include "mysocket.h"

int main(){
    int a=5,b=6;
    printf("The product of %d and %d is %d\n",a,b,mul(a,b));
    return 0;
}