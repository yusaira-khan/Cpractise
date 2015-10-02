//
// Created by yusaira-khan on 01/10/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(){
    int time = 30;
    sleep(time);
    printf("Slept for %d sec. Bye!\n",time);
    return 0;
}