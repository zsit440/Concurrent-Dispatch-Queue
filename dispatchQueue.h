/* 
 * File:   dispatchQueue.h
 * Author: robert
 *
 * Modified by: your UPI
 */

#ifndef DISPATCHQUEUE_H
#define	DISPATCHQUEUE_H

//#include "dispatchQueue.c"
#include <pthread.h>
#include <semaphore.h>
    
#define error_exit(MESSAGE)     perror(MESSAGE), exit(EXIT_FAILURE)

    typedef enum { // whether dispatching a task synchronously or asynchronously
        ASYNC, SYNC
    } task_dispatch_type_t;
    
    typedef enum { // The type of dispatch queue.
        CONCURRENT, SERIAL
    } queue_type_t;

    typedef enum { 
        RUNNING, WAITING
    } queue_status; //queue status
    
    typedef struct task task_t;

    struct task {
        char name[64];              // to identify it when debugging
        void (*work)(void *);       // the function to perform
        void *params;               // parameters to pass to the function
        task_dispatch_type_t type;  // asynchronous or synchronous
        task_t *prev; //task_pointer
        task_t *next; //next task pointer
        sem_t syn_sem; //next_task                
    };
    
    typedef struct dispatch_queue_t dispatch_queue_t; // the dispatch queue type
    typedef struct dispatch_queue_thread_t dispatch_queue_thread_t; // the dispatch queue thread type

    struct dispatch_queue_thread_t {
        dispatch_queue_t *queue;// the queue this thread is associated with
        pthread_t thread;       // the thread which runs the task
        sem_t thread_semaphore; // the semaphore the thread waits on until a task is allocated
        task_t *task;           // the current task for this tread
    };

    struct dispatch_queue_t {
        task_t * head;  //first task 
        task_t * tail;  // last task
        sem_t q_sem;   // running semaphore
        sem_t wait_sem; //dispatch sync semaphore
        pthread_t *threads; //thread pool for queue
        pthread_mutex_t lock; //lock for critical section
        int counter; //counter for task in the queue
        queue_type_t queue_type; 
                  // the type of queue - serial or concurrent
        queue_status status; //status of the queue
    };
    
    task_t *task_create(void (*work)(void *), void *params, char*name);

    
    void task_destroy(task_t* head);

    dispatch_queue_t *dispatch_queue_create(queue_type_t type);
    
    void dispatch_queue_destroy(dispatch_queue_t * myqueue);
    
    void dispatch_async(dispatch_queue_t *queue, task_t *my_task);
    
    void dispatch_sync(dispatch_queue_t *queue, task_t *my_task);
        
    void dispatch_queue_wait(dispatch_queue_t *queue); 

#endif	/* DISPATCHQUEUE_H */

