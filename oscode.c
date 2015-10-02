#define _GNU_SOURCE //Needed to use getline with c99

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define HISTORY_SIZE 10
#define ARGS_MAX 20
#define JOBS_MAX 10

struct job {
    char **command;
    int pid;
};

int get_index(int number) {
    return (number - 1) % HISTORY_SIZE;
}

void print_single_command(char **command) {
    int i = 0;

    while (command[i] != (char *) NULL) {
        fprintf(stdout, " %s", command[i]);
        i++;
    }
}

void copy_command(char **source_command, char **dest_command) {
    int j = 0;
    while (source_command[j] != (char *) NULL) {
        fflush(stdout);
        char *word = source_command[j];
        dest_command[j] = word;
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
        command = history[get_index(number)];
        fprintf(stdout, "%d ", number);
        print_single_command(command);
        fprintf(stdout, "\n");
    }
    fflush(stdout);
}

int getcmd(char *prompt, char *args[], int *background) {
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    fprintf(stdout, "%s", prompt);
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


void exec_arg(char *args[], int bg, int *exit_code_store,
              struct job jobs[], int *job_count) {
    int child_pid = fork();
    if (child_pid) {
        //Parent (process that runs loop)
        if (!bg) {//Process is not running in background so wait for it to end
            wait(exit_code_store);
        }
        else {
            //it's running in background so go ask for the next command
            //Store exit code as 1, can't restore the task
            *exit_code_store = 0;
            if (*job_count < JOBS_MAX) {
                struct job new = {args, child_pid};
                jobs[*job_count] = new;
                *job_count = *job_count + 1;
            }
        }
    } else {
        //child process(process that executes command)
        execvp(args[0], args);
        exit(-2);//program not found, so end with error
    }
}

void get_command_from_history(char *history[][ARGS_MAX], char x, int total_command_count, int *exit_codes,
                              struct job jobs[], int *job_count) {
    int end = total_command_count, start = 1;
    if (total_command_count > HISTORY_SIZE) {
        start = total_command_count - HISTORY_SIZE + 1;
    }
    int buffer_end_index = get_index(total_command_count);
    int exists = 0;
    char selection_char, **args = history[buffer_end_index];
    fflush(stdout);
    for (int number = end - 1; number >= start; number--) {
        int i = get_index(number);
        if (args[1] == (char *) NULL) {
            exists = 1;
        } else {
            selection_char = args[1][0];
            if (history[i][0][0] == selection_char) {
                exists = 1;
            } else {
                exists = 0;
            }
        }
        if (exists) {
            copy_command(history[i], args);
            print_single_command(args);
            fprintf(stdout, "\n");
            exit_codes[buffer_end_index] = exit_codes[i];
            if (exit_codes[i] != 0) {
                fprintf(stderr, "Command exited with error code %d before so will not execute again\n", exit_codes[i]);
                exit_codes[buffer_end_index] = 1;
            } else {
                exec_arg(args, 0, &exit_codes[buffer_end_index], jobs, job_count);
            }
            return;
        }
    }
}

void print_jobs(struct job jobs[], int *job_count) {
    int  child_status_temp;
    for (int i = 0; i < *job_count; i++) {
        int status = waitpid(jobs[i].pid, &child_status_temp, WNOHANG);
        if (status == 0) {
            printf("ID: %d\tBackground command: ", jobs[i].pid);
            print_single_command(jobs[i].command);
            printf("\n");

        } else {
            //FIXME: shift jobs

            *job_count = *job_count - 1;
            i--;
        }
    }
    if (*job_count == 0) {
        printf("No Background Jobs.");
    }
}

void execute_foreground(char *args[], struct job jobs[], int *job_count) {
    int child_status_temp, selected_job_pid;
    selected_job_pid = atoi(args[1]);
    waitpid(selected_job_pid, &child_status_temp, 0);
    *job_count = *job_count - 1;
    //FIXME: shift jobs
}

int exec_builtin(char *history[][ARGS_MAX], int exit_code_store[], int total_command_count,
                 struct job jobs[], int *job_count) {
    char **args = history[get_index(total_command_count)];
    exit_code_store[get_index(total_command_count)] = -1;//not repeatable via 'r' command

    if (strcmp(args[0], "history") == 0) {
        print_history(history, total_command_count);
        return 1;
    } else if (strcmp(args[0], "r") == 0) {
        get_command_from_history(history, '\0', total_command_count, exit_code_store, jobs, job_count);
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
        print_jobs(jobs, job_count);
        return 1;

    } else if (strcmp(args[0], "fg") == 0) {
        if (args[1] == (char *) NULL) {
            fprintf(stderr, "No PID given for fg");
            exit_code_store[get_index(total_command_count)] = 1; //NO PID provided
        }
        else{
            execute_foreground(args,jobs,job_count);
        }
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0) {
        exit(-1);
    }
    return 0;//command is not builtin
}

int main() {
    setbuf(stdout, NULL);
    char **args; //Array of strings, to hold each argument of a command (Array of pointers not pointer to array)
    int bg; //Boolean to indicate that command should run in background
    int always = 1;

    //Variables for storing commands
    int args_count = 0, command_count = 0, index = -1, exit_codes[HISTORY_SIZE];
    char *history[HISTORY_SIZE][ARGS_MAX];
    int command_is_builtin = 0;//Boolean to indicate whether command was builtin or is a program

    struct job jobs_list[JOBS_MAX];
    int job_count = 0;

    while (always) {
        index = get_index(command_count + 1);
        args = history[index];
        args_count = getcmd("\n>>  ", args, &bg);
        args[args_count] = (char *) NULL;
        command_count++;
        command_is_builtin = exec_builtin(history, exit_codes, command_count, jobs_list, &job_count);
        if (!command_is_builtin) {
            exec_arg(args, bg, &exit_codes[index], jobs_list, &job_count);
        }
    }
    return 0; //IDE complains
}
