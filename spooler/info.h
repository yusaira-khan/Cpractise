//
// Created by yusaira-khan on 18/10/15.
//

#ifndef SPOOLER_INFO_H
#define SPOOLER_INFO_H
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define buffer_shared_mem_name "assignment2_buffer"
#define job_list_shared_mem_name "assignment2_jobs"


struct job {
    int duration;//In seconds
    int client_id;
    int job_id;
    int buffer_index;

};

struct buffer{
/*    struct job* jobs_list;*/ //TO BE MEMORY SAFE
    int read_index, write_index;
    int max_size;
    sem_t full_lock;
    sem_t empty_lock;
    sem_t access_lock;

};

typedef  struct buffer * Buffer;
typedef struct job *Buffer_Jobs;
#endif //SPOOLER_INFO_H
