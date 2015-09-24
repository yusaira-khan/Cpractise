#define _GNU_SOURCE //Needed to use getline with c99

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define HISTORY_SIZE 11
#define ARGS_MAX 20

/**
 * Handles command input
 * @param prompt Token  to be displayed to indicate start of new command
 * @param args Placeholder Array of strings where each argument of the command will be returned
 * @param background Pointer to Boolean  to indicate if command should run in background
 */
inline int get_index(int number) {
    return (number - 1) % HISTORY_SIZE;
}

void get_history(char ***history, int total_command_count) {
    int hist_start = 1, hist_end = total_command_count;
    char** command;
    if (total_command_count > HISTORY_SIZE) {
        hist_start = total_command_count - HISTORY_SIZE + 1;
    }
    for (int number = hist_start; number < hist_end; number++) {
        printf("%d",number);
        command =  history[get_index(number)];
    }
}

void get_command(char*** history, int* exit_codes){

}


int getcmd(char *prompt, char *args[], int *background) {
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    printf("%s", prompt);
    fflush(stdin);
    length = getline(&line, &linecap, stdin);

    //CTRL-D pressed
    if (length <= 0) {
        printf("\b\b\b\bBye!\n");
        exit(0);
    }

    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }

    return i;
}

void exec_arg(char *args[], int bg) {
    int child_status;
    if (fork()) {
        //Parent (process that runs loop)
        if (!bg)//Process is not running in background so wait for it to end
            wait(&child_status);
        //else, it's running in background so go ask for the next command
    } else {
        //child process(process that executes command)
        execvp(args[0], args);
        exit(-2);//program not found, so end with error
    }
}

int main() {
    char **args; //Array of strings, to hold each argument of a command (Array of pointers not pointer to array)
    int bg; //Boolean to indicate that command should run in background
    int always = 1;
    char *history[HISTORY_SIZE][ARGS_MAX];
    //int *bg_history[HISTORY_SIZE];
    int command_count = 0;

    while (always) {
        args = history[get_index(command_count + 1)];
        getcmd("\n>>  ", args, &bg);
        command_count++;
        exec_arg(args,bg);

    }
    return 0; //IDE complains
}


