#include <pthread.h>
#include <semaphore.h>
#include "dispatchQueue.h"
#include <stdlib.h>
#include  <string.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

void* task_runner(void* queue){
	dispatch_queue_t * myqueue = (dispatch_queue_t *)queue; // cast the type of the queue

	while (1){
		if(myqueue -> status == WAITING && myqueue -> counter  <= 0){
			sem_post(&(myqueue-> wait_sem)); //Wake up the main thread in dispatch_Queue_Wait()
			myqueue ->status = RUNNING; //unblock the queue


		}
		sem_wait(&(myqueue->q_sem));


			pthread_mutex_lock(&(myqueue->lock));
			task_t *my_task = (task_t*)myqueue -> head; // get the task from the queue
			myqueue -> head = myqueue -> head  -> next;
			my_task -> next == NULL;
			//printf("1\n");
			pthread_mutex_unlock(&(myqueue->lock));
			my_task->work(my_task -> params);	

		if(my_task->type == SYNC){
			sem_post(&(my_task-> syn_sem)); //wake the main thread in dispatch sync
		}
		task_destroy(my_task);
		pthread_mutex_lock(&(myqueue->lock)); //avoid mutiple counter locked
		myqueue -> counter --;
		pthread_mutex_unlock(&(myqueue->lock));
		

		

	}

};

dispatch_queue_t *dispatch_queue_create(queue_type_t type){ //function to create queue
	dispatch_queue_t * queue  = (dispatch_queue_t *)malloc(sizeof (dispatch_queue_t)); //allocate memory to a queue and convert to queue pointer
	queue -> head = NULL;
	queue -> tail = NULL;
	queue -> queue_type = type;
	queue -> counter = 0;
	int num_cores = get_nprocs();
	if (type == SERIAL){
		num_cores = 1;
	}
	queue -> threads = (pthread_t*)malloc(sizeof(pthread_t) * num_cores); //allocate thread pool size
	sem_init(&(queue->q_sem),0 ,0);
	sem_init(&(queue->wait_sem),0 ,0);
	for (int i = 0; i < num_cores; i++){

		pthread_create(&(queue->threads)[i], NULL, (void *)task_runner,(void *)queue); //fire the number of threads according to the demand
		//printf("yes");
	}
	
	return queue;

};

void dispatch_queue_destroy(dispatch_queue_t * myqueue){
	task_t* curr = myqueue -> head;
	task_t* next;
	while(curr !=NULL){
		free(curr);
		curr = curr -> next; //free all the task pointer
	}

	free(myqueue -> threads);
	sem_destroy(&(myqueue->q_sem));
	sem_destroy(&(myqueue->wait_sem));
	free(myqueue);
};    

task_t *task_create(void (*work)(void *), void *params, char*name){
	task_t *task = (task_t *)malloc(sizeof (task_t));
    	task->work = work; // (*task).work = work;
    	task->params = params;
    	strcpy(task->name, name);
    	return task;
};

void add_task(dispatch_queue_t *queue, task_t *my_task){
	if (queue->status != WAITING){
	if (queue -> head == NULL && (queue->tail) == NULL){
		queue -> head = queue->tail = my_task;
	}
	else{
	queue->tail->next = my_task;
	queue->tail = my_task;
}
	my_task -> next = NULL;
	queue -> counter ++; //add a counter to the queue when a task is added
}
};


void dispatch_async(dispatch_queue_t *queue, task_t *my_task){
	my_task -> type = ASYNC;
	
	add_task(queue, my_task);// add the task to the queue
	sem_post(&(queue-> q_sem)); //signal the thread a task has been added

};

void task_destroy(task_t* head){
	//sem_destroy(&(head->syn_sem));
	free(head);
}

void dispatch_sync(dispatch_queue_t *queue, task_t *my_task){
	my_task -> type = SYNC;
	sem_init(&(my_task->syn_sem),0 ,0); //initilaised a syn flag
	add_task(queue, my_task);
	sem_post(&(queue-> q_sem));
	sem_wait(&(my_task-> syn_sem)); //wait for the queue to finish processing the task
	
};
	


void dispatch_queue_wait(dispatch_queue_t *queue) {
	queue -> status = WAITING; //change the queue status to waiting.
	sem_wait(&(queue-> wait_sem));
	//sem_post(&(queue-> q_sem));

};