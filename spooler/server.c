#include <signal.h>
#include "info.h"

sem_t *exit_lock = NULL;

void bye(int sig) {
    if (exit_lock)
        sem_wait(exit_lock);

    shm_unlink(buffer_shared_mem_name);
    shm_unlink(job_list_shared_mem_name);
    printf("Bye!\n");
    //munmnap happens on process exit
    exit(EXIT_SUCCESS);
}


void setup_shared_mem(Buffer *spool, Buffer_Jobs *buffer_jobs, int max_size) { // create a shared memory segment
    int fd_buffer, fd_jobs, buff_alloc, jobs_alloc, i;
    Buffer_Jobs jobs_list = NULL;

    assert(*spool == NULL);
    fd_buffer = shm_open(buffer_shared_mem_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd_buffer == -1 && errno == EEXIST) {
        fprintf(stderr, "Printer is already running! Close printer and try again\n");
        bye(SIGQUIT);
    }

    fd_jobs = shm_open(job_list_shared_mem_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd_buffer == -1 && errno == EEXIST) {
        fprintf(stderr, "Printer is already running or didn't close properly. Try running this command again\n");
        shm_unlink(buffer_shared_mem_name);
        bye(SIGTERM);
    }

    buff_alloc = sizeof(**spool);
    jobs_alloc = (int) sizeof(*jobs_list) * max_size;
    ftruncate(fd_buffer, buff_alloc);
    ftruncate(fd_jobs, buff_alloc);

    *spool = (Buffer) mmap(NULL, buff_alloc, PROT_READ | PROT_WRITE, MAP_SHARED, fd_buffer, 0);

    jobs_list = (Buffer_Jobs) mmap(NULL, jobs_alloc, PROT_READ | PROT_WRITE, MAP_SHARED, fd_jobs, 0);
    *buffer_jobs = jobs_list;

    for (i = 0; i < max_size; i++) {
        jobs_list[i].buffer_index = i;
        jobs_list[i].duration = 0;
        jobs_list[i].client_id = 0;
        jobs_list[i].job_id = 0;
    }

    (*spool)->read_index = 0;
    (*spool)->write_index = 0;
    (*spool)->max_size = max_size;
    *buffer_jobs = jobs_list;
}


void init_semaphore(Buffer buff, int max_size) {// initialize the semaphore and put it inshared memory
    sem_init(&(buff->access_lock), 1, 1); //Doesn't allow simultaeneous access
    sem_init(&(buff->empty_lock), 1, 0); //Blocks reading an empty buffer, maxes at max_size
    sem_init(&(buff->full_lock), 1, max_size); //Blocks reading an full buffer, maxes at max_size
}


Buffer_Jobs take_a_job(Buffer buff, Buffer_Jobs *buffer_jobs) {// this is blocking on a semaphore if no job
    int read_index, sem_value;


    /*Since empty_lock semaphore is 0 initially, and can go up to buff->max_size (before full_lock is locked),
     * this empty_lock checks for buffer underflow and blocks reading anmpty buffer*/
    sem_getvalue(&buff->empty_lock, &sem_value);
    if (sem_value == 0) {
        printf("No request in buffer, Printer sleeps\n");
        fflush(stdout);
    }
    sem_wait(&(buff->empty_lock));

    sem_wait(&(buff->access_lock));//Don't let new clients access the buffer till take_a_job is completed

    read_index = buff->read_index;
    Buffer_Jobs j = &((*buffer_jobs)[read_index]);

    //Increment read index so that the data at that index is read next time
    buff->read_index = (read_index + 1) % buff->max_size;
    sem_post(&(buff->access_lock));
    sem_post(&(buff->full_lock));

    return j;
}


void print_a_msg(Buffer_Jobs current, int start) { // duration of job, job ID, source of job are printed
    char *meh;
    if (start) {
        meh = "starts";
    } else {
        meh = "finishes";
    }
    printf("Printer %s printing %d pages from Buffer[%d] (ClientID: %d, JobID: %d).\n", meh,
           current->duration, current->buffer_index, current->client_id, current->job_id);
    fflush(stdout);
}

void signal_exits() {//All the exit signals
    int signals[] = {SIGHUP, SIGINT, SIGSEGV, SIGQUIT, SIGTERM};
    int i;
    for (i = 0; i < 5; i++) {
        signal(signals[i], bye);
    }
}


int main(int argc, char **argv) {
    int buff_length;
    int always = 1, job_id = 0;
    Buffer spooler = NULL;
    Buffer_Jobs buffer_jobs = NULL, current_job = NULL;

    if (argc < 2) {
        fprintf(stderr, "You didn't specify a spool size.\nExiting.\n");
        exit(-1);
    }

    signal_exits();
    buff_length = atoi(argv[1]);
    setup_shared_mem(&spooler, &buffer_jobs, buff_length); // create a shared memory segment
    assert(spooler != NULL);

    printf("Printer created with a buffer of %d.\n", buff_length);
    init_semaphore(spooler, buff_length); // initialize the semaphore and put it inshared memory

    exit_lock = &(spooler->access_lock);

    while (always) {
        job_id++;
        current_job = take_a_job(spooler, &buffer_jobs); // this is blocking on a semaphore if no job
        current_job->job_id = job_id;
        print_a_msg(current_job, 1); // duration of job, job ID, source of job are printed
        sleep((uint) current_job->duration); // sleep for job duration
        print_a_msg(current_job, 0); // duration of job, job ID, source of job are printed
    }

    return -1;//It won't get to this
}