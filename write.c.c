//
// Created by yusaira-khan on 02/10/15.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int i = 1000000;
    time_t t;
    FILE *fp;
    char filename[50], writing[100];

    /* Intializes random number generator */
    srand((unsigned) time(&t));
    sprintf(filename, "/test/File_NO_%d", rand());
    printf("file %s", filename);
    i= i + rand();
    for (; i > 0; i--) {
        fp = fopen(filename, "a+");
        fprintf(fp,"Hello, %s! There are %d lines left to write\n", filename, i);
        fclose(fp);
        fflush(fp);
//        sleep(10);

    }


}
