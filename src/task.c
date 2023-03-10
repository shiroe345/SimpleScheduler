#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "../include/task.h"
#include "../include/scheduler.h"

void task_sleep(int ten_ms)
{   
    if(ready_queue.size > 1 && !running->next) exit(1);

    //printf("%d ",ten_ms);
    printf("Task %s goes to sleep.\n", running->name);
    running->time_to_wait = ten_ms;
    running->sleeping = 1;

    //if(!running->next) printf("heelo");
    
    /* delete running node in ready_queue */
    if(ready_queue.size > 1){ //ready_queue's size > 1 , same as if(running -> next)
        ready_queue.head = running->next;
        ready_queue.head->pre = NULL;
    }
    else{
        // printf("aloi");
        // if(running->next){
        //     printf("%s %s\n",running->next->name,running->next->another_me->state);
        //     task_node* tttt = waiting_queue.head;
        //     while(tttt){
        //         printf("%s ",tttt->name);
        //         tttt = tttt->next;
        //     }
        //     printf("\n");
        //     tttt = ready_queue.head;
        //     while(tttt){
        //         printf("%s ",tttt->name);
        //         tttt = tttt->next;
        //     }
        // }
        // printf("\n");
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

    task_node* tmp = running;
    running = NULL;

    /* save context here */
   
    swapcontext(&tmp->ctx,&main_thread);
    
}

void task_exit()
{
    task_pool.terminate_num++;
    sprintf(running->another_me->state,"TERMINATED");
    running->another_me->turnaround_time = running->turnaround_time;

    /* delete running node in ready_queue */
    if(running->next){ //ready_queue's size > 1
        ready_queue.head = running->next;
        ready_queue.head->pre = NULL;
    }
    else{
        ready_queue.head = NULL;
        ready_queue.tail = NULL;
    }
    ready_queue.size--;

    printf("Task %s has terminated.\n", running->name);
    task_node* tmp = running;
    running = NULL;
    swapcontext(&tmp->ctx,&main_thread);
    // ucontext_t tmp;
    // running = NULL;
    // swapcontext(&tmp,&main_thread);
}

int cts(){ //count ready queue size
    task_node *tmp = ready_queue.head;
    int sum = 0;
    while(tmp){
        sum++;
        tmp = tmp -> next;
    }
    return sum;
}


int cts2(){
    task_node *tmp = ready_queue.tail;
    int sum = 0;
    while(tmp){
        sum++;
        tmp = tmp -> pre;
    }
    return sum;
}