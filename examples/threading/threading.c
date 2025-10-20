#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEBUG_LOG(msg,...) printf("Threading EVENT: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("Threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    int rc = 0;
    struct timespec ts1, ts2;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    ts1.tv_sec = 0;
    ts1.tv_nsec = (thread_func_args->wait_to_obtain_ms) * 1000000UL;
    ts2.tv_sec = 0;
    ts2.tv_nsec = (thread_func_args->wait_to_release_ms) * 1000000UL;
    printf("Values - Wait to Obtain: %ld, Wait to Release: %ld\n", ts1.tv_nsec, ts2.tv_nsec);
    nanosleep(&ts1, NULL);
    rc = pthread_mutex_lock(thread_func_args->mutex);
    if(rc != 0)
    {
	printf("Mutex lock failed with error: %d\n", rc);
	thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    nanosleep(&ts2, NULL);
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if(rc != 0)
    {
	printf("Mutex unlock failed with error: %d\n", rc);
	thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    thread_func_args->thread_complete_success = true;
    return thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    int rc = 0;
    struct thread_data *params = malloc(sizeof(struct thread_data));
    if(!params)
    {
	printf("Memory allocation failed\n");
	return false;
    }
    params->thread_complete_success = false;
    params->wait_to_obtain_ms = wait_to_obtain_ms;
    params->wait_to_release_ms = wait_to_release_ms;
    params->mutex = mutex;
    rc = pthread_create(thread, NULL, threadfunc, (void *)params);
    if(rc != 0)
    {
	printf("Thread creation failed with error: %d\n", rc);
        free(params);
	return false;
    }

    return true;
}

