#include <stdio.h>
#include <unistd.h>

int main() {

    int i;
    i = 10;
    printf("Hellow Worlds %d",i);
    fflush(stdout);
    if(fork() == 0){
        i+=10;
    }
    printf(" %d ",i);
    printf(" My process ID : %d\n", getpid());
    fflush(stdout);
}