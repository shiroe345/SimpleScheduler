#include "../include/resource.h"
#include "../include/task.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void get_resources(int count, int *resources)
{
    running->number_of_resource = count;
    for(int i = 0;i<count;i++)
        running->resources[i] = resources[i];

    getcontext(&running->ctx);
    for(int i = 0;i<count;i++){ //check if all resource available
        if(shared_resources[resources[i]]){ //context switch here
            if(ready_queue.size > 1){ //ready_queue's size > 1 , same as if(running -> next)
                ready_queue.head = running->next;
                ready_queue.head->pre = NULL;
            }
            else{
                ready_queue.head = NULL;
                ready_queue.tail = NULL;
            }
            ready_queue.size--;

            /* add it to waiting_queue */
            if(waiting_queue.size > 0){
                waiting_queue.tail->next = running;
                running->pre = waiting_queue.tail;
                waiting_queue.tail = running;
                waiting_queue.tail->next = NULL;
            }
            else{
                waiting_queue.head = running;
                waiting_queue.head->pre = NULL;
                waiting_queue.tail = running;
                waiting_queue.tail->next = NULL;
            }
            waiting_queue.size++;

            sprintf(running->another_me->state,"WAITING");
            printf("Task %s is waiting resource.\n",running->name);
            running = NULL;
            
            setcontext(&main_thread);
        }
    }   

    running->another_me->resource[0] = '\0';
    
    for(int i = 0;i<count;i++){
        running->resources[i] = resources[i];
        shared_resources[resources[i]] = running->tid;
        printf("Task %s gets resource %d.\n",running->name,resources[i]);
        char tmp[3];
        sprintf(tmp," %d",resources[i]);
        strcat(running->another_me->resource, tmp);
    }
}

void release_resources(int count, int *resources)
{
    running->number_of_resource = 0;
    for(int i = 0;i<count;i++){
        printf("Task %s releases sesource %d.\n",running->name,resources[i]);
        shared_resources[resources[i]] = 0;
    }
    sprintf(running->another_me->resource,"none");

}
