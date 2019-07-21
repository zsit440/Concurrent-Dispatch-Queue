# Concurrent-Dispatch-Queue

The main idea of this queue is to allocate threads from a thread pool to process tasks efficiently. This queue can ultilise all the cores in the CPU to process mutiple tasks at the same time. The default setting of this queue one thread per core.

## Function explanation

```C
dispatch_queue_t *dispatch_queue_create(queue_type_t queueType) 
```

Initilaize a dispatch queue and create a thread pools to store the number of cores.

```C
void dispatch_queue_destroy(dispatch_queue_t *queue) //
```

Destroys the dispatch queue. All allocated memory and resources such as semaphores are
released and returned.

```C
task_t *task_create(void (* work)(void *), void *param, char* name)
```

Creates a task. work is the function to be called when the task is executed, param is a pointer to
either a structure which holds all of the parameters for the work function to execute with or a single
parameter which the work function uses. If it is a single parameter it must either be a pointer or
something which can be cast to or from a pointer. The name is a string of up to 63 characters. This
is useful for debugging purposes.
Returns: A pointer to the created task.

```C
void task_destroy(task_t *task)
```

Destroys the task. Call this function as soon as a task has completed. All memory allocated to the
task should be returned.

```C
void dispatch_sync(dispatch_queue_t *queue, task_t *task)
```

Sends the task to the queue (which could be either CONCURRENT or SERIAL). This function does
not return to the calling thread until the task has been completed

```C
void dispatch_sync(dispatch_queue_t *queue, task_t *task)
```

Sends the task to the queue (which could be either CONCURRENT or SERIAL). This function does
not return to the calling thread until the task has been completed

```C
void dispatch_queue_wait(dispatch_queue_t *queue)
```
Waits (blocks) until all tasks on the queue have completed. If new tasks are added to the queue
after this is called they are ignored.

## Main idea
I used a linked list queue for this Queue. Here is how the assignment works.
First, I created a function called dispatch_queue_create to assign head, tail pointer and queue type to the queue. This program fires up a number of threads according to the thread demand. If this queue is a serial queue, it only fires up one thread and only one task will be grabbed frome the queue, otherwise it fires up the number of threads equals to the number of cores and multiple tasks are executed at the same time.The p_thread_create function invokes a task runner function by passing in parameter queue.


Next, I used a casting to change the queue from void back to a queue pointer, so I could extract the item from queue. This while loop purpose is to keep the loop awakening. Initially, this loop is paused by sem_wait(q_sem) since there isn’t any task in the queue. The semaphore value is zero. This loops only runs when there is at least one task in the queue.
The pthread_lock is used to stop the threads getting the same task from the queue and over subtracting the counter.

For the dispatch_async and dispatch_sync function, I simply add the tasks to queue and increase the count of q_sem when a task enters the queue to wake up the loop. The difference is in dispatch sync, the line sem_wait is used to pause the main thread. When a task is finished, the task runner checks whether a task is synchronous, if it is, the main thread in the dispatch sync function will be running again.
