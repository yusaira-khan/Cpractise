#define _GNU_SOURCE //Needed to use getline with c99

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define HISTORY_SIZE 10
#define ARGS_MAX 20

int get_index(int number) {
    return (number - 1) % HISTORY_SIZE;
}

void print_single_command(char **command) {
    int i = 0;

    while (command[i] != (char *) NULL) {
        printf("%s ", command[i]);
        i++;
    }
}

void copy_command(char **source_command, char **dest_command) {
    int j = 0;
    while (source_command[j] != (char *) NULL) {
        char *word = source_command[j], *dest = dest_command[j];
        int k = 0;
        while (word[k] != '\0') {
            dest[k] = word[k];
            k++;
        }
        dest[k] = '\0';
        j++;
    }
    dest_command[j] = (char *) NULL;
}

void print_history(char *history[][ARGS_MAX], int total_command_count) {
    int hist_start = 1, hist_end = total_command_count;
    char **command;
    if (total_command_count > HISTORY_SIZE) {
        hist_start = total_command_count - HISTORY_SIZE + 1;
    }

    for (int number = hist_start; number <= hist_end; number++) {
        printf("%d ", number);
        command = history[get_index(number)];
        print_single_command(command);
        printf("\n");
    }
    fflush(stdout);
}

int getcmd(char *prompt, char *args[], int *background) {
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    //CTRL-D pressed
    if (length <= 0) {

        exit(-1);
    }
    fflush(stdout);
    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;
    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++)
            //Add breaks at the end of each argument
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    return i;
}


void exec_arg(char *args[], int bg, int *exit_code_store) {
    if (fork()) {
        //Parent (process that runs loop)
        if (!bg) {//Process is not running in background so wait for it to end
            wait(exit_code_store);
        }
        else {
            //it's running in background so go ask for the next command
            //Store exit code as 1, can't restore the task
            *exit_code_store = 0;
        }
    } else {
        //child process(process that executes command)
        execvp(args[0], args);
        exit(-2);//program not found, so end with error
    }
}

void get_command_from_history(char *history[][ARGS_MAX],
                              char x, int total_command_count, int *exit_codes) {
    int end = total_command_count, start = 1;
    if (total_command_count > HISTORY_SIZE) {
        start = total_command_count - HISTORY_SIZE + 1;
    }
    int buffer_end_index = get_index(total_command_count);
    //FIXME:handle errors
    //FIXME: Doesn't work
    fflush(stdout);
    for (int number = end-1; number >= start; number--) {
        int i = get_index(number);
        if (1) {
            copy_command(history[i], history[buffer_end_index]);
            print_single_command(history[buffer_end_index]);
            printf("\n");
            if (exit_codes[i] != 0) {
                fprintf(stderr, "Command did not execute properly before so will not execute again\n");
                exit_codes[buffer_end_index] = 1;
            } else {
                exec_arg(history[buffer_end_index], 0, &exit_codes[buffer_end_index]);
            }
            return;
        }
    }
}

int exec_builtin(char *history[][ARGS_MAX], int exit_code_store[], int total_command_count) {
    char **args = history[get_index(total_command_count)];
    if (strcmp(args[0], "history") == 0) {
        print_history(history, total_command_count);

        return 1;
    } else if (strcmp(args[0], "r") == 0) {
        get_command_from_history(history, '\0', total_command_count, exit_code_store);
        return 1;
    } else if (strcmp(args[0], "cd") == 0) {
        chdir(args[1]);
        return 1;
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            fprintf(stdout, "Current working directory: %s\n", cwd);
        return 1;
    }
    else if (strcmp(args[0], "jobs") == 0) {
        return 1;
    } else if (strcmp(args[0], "fg") == 0) {
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0) {
        exit(-1);
    }
    exit_code_store[get_index(total_command_count)] = 0;
    return 0;
}

int main() {
    setbuf(stdout, NULL);
    char **args; //Array of strings, to hold each argument of a command (Array of pointers not pointer to array)
    int bg; //Boolean to indicate that command should run in background
    int always = 1;
    int args_count = 0;
    char *history[HISTORY_SIZE][ARGS_MAX];
    int exit_codes[HISTORY_SIZE];
    int command_count = 0;
    int index = -1;
    int command_is_builtin = 0;
    //TODO: share memory between processes for jobs and fg (share process id and remove when finished)
    while (always) {
        index = get_index(command_count + 1);
        args = history[index];
        args_count = getcmd("\n>>  ", args, &bg);
        args[args_count] = (char *) NULL;
        command_count++;
        command_is_builtin = exec_builtin(history, exit_codes, command_count);
        if (!command_is_builtin) {
            exec_arg(args, bg, &exit_codes[index]);
        }
    }
    return 0; //IDE complains
}
