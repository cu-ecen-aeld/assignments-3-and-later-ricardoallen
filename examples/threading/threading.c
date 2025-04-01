#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    // wait, obtain mutex, wait, release mutex as described by thread_data structure
    struct thread_data *thread_func_args = (struct thread_data *) thread_param;
    pthread_mutex_t *mutex = thread_func_args->mutex;
    int wait_o_ms = thread_func_args->wait_to_obtain_ms;
    int wait_r_ms = thread_func_args->wait_to_release_ms;
    bool *success = &(thread_func_args->thread_complete_success);
    // usleep is microseconds but wait_o_ms is in miliseconds so multiply by 1000
    usleep(wait_o_ms * 1000);
    if(pthread_mutex_lock ( mutex)==0){
        usleep(wait_r_ms * 1000);
        if(pthread_mutex_unlock ( mutex)==0){
           *success = true;
        }
    }
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{   
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data *my_thread_data=malloc(sizeof(struct thread_data));

    my_thread_data->pthread_info = thread;
    my_thread_data->mutex = mutex;
    my_thread_data->wait_to_obtain_ms = wait_to_obtain_ms;
    my_thread_data->wait_to_release_ms = wait_to_release_ms;
    my_thread_data->thread_complete_success = false;
    int ret = pthread_create(thread, NULL, threadfunc, (void *) my_thread_data);
    if (ret) {
        return false;
    }
    
    return true;
}

