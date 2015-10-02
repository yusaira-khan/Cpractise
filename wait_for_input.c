//
// Created by yusaira-khan on 01/10/15.
//

#include <stdio.h>
#include <stdlib.h>
int main(){

    char *line;
    size_t linecap = 0;
    int length, i = 0;
    fprintf(stdout, "Tell me something: ");
    length = getline(&line, &linecap, stdin);
    printf("You said: \t%sBye!\n",line);
    return 0;
}