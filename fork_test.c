#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int main() {
int stat;
    char*yo[10];
    yo[0]="cat";
    yo[1]="1";
    yo[2]= (char *) NULL;

  if(fork()==0){

      execvp(yo[0],yo);
      exit(-5);
  } else{
      wait(&stat);
      printf("%d",stat);
  }
}