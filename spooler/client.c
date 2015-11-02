#include "info.h"

void attach_share_mem(Buffer *spool, Buffer_Jobs *bufferJobs) {
    int fd_buffer, fd_jobs, buff_alloc, jobs_alloc;

    assert(*spool == NULL);
    fd_buffer = shm_open(buffer_shared_mem_name, O_RDWR, 0600);
    fd_jobs = shm_open(job_list_shared_mem_name, O_RDWR, 0600);

    buff_alloc = sizeof(**spool);
    *spool = mmap(NULL, buff_alloc, PROT_READ | PROT_WRITE, MAP_SHARED, fd_buffer, 0);
    jobs_alloc = (int) sizeof(**bufferJobs) * (*spool)->max_size;
    *bufferJobs = mmap(NULL, jobs_alloc, PROT_READ | PROT_WRITE, MAP_SHARED, fd_jobs, 0);
}

void put_a_job(Buffer buff, Buffer_Jobs safe_buffer_jobs, int client_id, int duration) {
    int write_index, was_full = 0, sem_value;

    /*full_lock semaphore is initially buff->max_size and when it is locked,  empty_lock semaphore is max*/
    sem_getvalue(&buff->full_lock, &sem_value);
    if (sem_value == 0) {
        was_full = 1;
        printf("Client %d has %d pages to print, buffer full, sleeps.\n", client_id, duration);
        fflush(stdout);
    }
    sem_wait(&buff->full_lock);


    sem_wait(&buff->access_lock);//Don't let printer or new clients access the buffer till put_a_job is completed
    write_index = buff->write_index;
    Buffer_Jobs j = &safe_buffer_jobs[write_index];
    j->client_id = client_id;
    j->duration = duration;

    if (was_full) {
        printf("Client %d wakes up, puts request in Buffer[%d]\n", client_id, j->buffer_index);
    } else {
        printf("Client %d has %d pages to print, puts request in Buffer[%d]\n", client_id, duration, j->buffer_index);
    }
    fflush(stdout);
    //Increment write index to prevent overwriting
    write_index = (write_index + 1) % buff->max_size;
    buff->write_index = (write_index);

    sem_post(&buff->access_lock);
    sem_post(&buff->empty_lock); //Removes empty waits or makes empty further away
}

void release_share_mem(Buffer spool, Buffer_Jobs bufferJobs) {
    int buff_alloc = sizeof(*spool), jobs_alloc;
    jobs_alloc = (int) sizeof(*bufferJobs) * (spool)->max_size;
    munmap(spool, buff_alloc);
    munmap(bufferJobs, jobs_alloc);
}

int main(int argc, char **argv) {
    Buffer spooler = NULL;
    Buffer_Jobs safe = NULL;
    int client_id, duration;
    if (argc != 3) {//Getting job params from args
        fprintf(stderr, "To use: client <client_id> <duration>\n");
        exit(EXIT_FAILURE);
    }
    client_id = atoi(argv[1]);
    duration = atoi(argv[2]);

    attach_share_mem(&spooler, &safe);//attaching memory segment
    // use the same key as the server so that the client can connect to the same memory segment
    assert(spooler != NULL);

    put_a_job(spooler, safe, client_id, duration); // create the job record and wait if the buffer is full


//release_share_mem(spool,safe); // release the shared memory
//Memory mappings are automatically released at the end of a process
    return EXIT_SUCCESS;
}