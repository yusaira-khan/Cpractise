#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main() {

    sleep(5);
    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);
    puts(buffer);

    return 0;
}