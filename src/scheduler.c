#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "../include/task.h"
#include "../include/scheduler.h"
#include "../include/function.h"
#include "../include/resource.h"
#include <assert.h>


void check_waiting_queue(){
	// printf("%d %d\n",ready_queue.size,waiting_queue.size);
	// printf("wf%d %d %d\n",ready_queue.size,cts(),cts2());
	task_node *tmp = waiting_queue.head;
	task_node *t;
	while(tmp){
		t = tmp->next;
		if(tmp->sleeping) tmp->time_to_wait--;
		else{
			int b = 1;
			for(int i = 0;i<tmp->number_of_resource;i++){ //check if resources available
				if(shared_resources[tmp->resources[i]]){ //resources still not available
					//printf("nono %s %d\n",tmp->name,tmp->resource[i]);
					b = 0;
					break;
				}
			}
			if(b){ //let it enter next if
				tmp->time_to_wait = 0;
				tmp->sleeping = 1;
			}
		}

		if(tmp->time_to_wait == 0 && tmp->sleeping){ //ste == 1 mean sleep
			tmp->sleeping = 0;
			sprintf(tmp->another_me->state,"READY");
			/* delete from waiting_queue */
			if(tmp == waiting_queue.head){
				waiting_queue.head = tmp->next;
				if(waiting_queue.head) waiting_queue.head->pre = NULL;	
			}
			if(tmp == waiting_queue.tail){
				waiting_queue.tail = tmp->pre;
				if(waiting_queue.tail) waiting_queue.tail->next = NULL;
			}
			if(tmp->pre) tmp->pre->next = tmp->next;
			if(tmp->next) tmp->next->pre = tmp->pre;
			waiting_queue.size--;
			
			/* add to ready_queue */
			if(ready_queue.size == 0){
				ready_queue.head = tmp;
				ready_queue.tail = tmp;
			}
			else{
				ready_queue.tail->next = tmp;
				tmp->pre = ready_queue.tail; //this line cost me 5 hours
				ready_queue.tail = tmp;
			}
			ready_queue.tail->next = NULL; //find this line cost me 3 hours!
			ready_queue.head->pre = NULL;

			ready_queue.size++;
			// printf("%s wake up %d %d\n",tmp->name,ready_queue.size,waiting_queue.size);
		}
		tmp = t; //change back to current node in waiting_queue if need;
	}
}

void rescheduling(){ //check waiting_queue and rescheduling
	// assert(ready_queue.size == 0 || ready_queue.head != NULL);
	check_waiting_queue(); //need to check waiting queue first
	// assert(ready_queue.size == 0 || ready_queue.head != NULL);
	// task_node* tttt = waiting_queue.head;
	// while(tttt){
	// 	printf("%s ",tttt->name);
	// 	tttt = tttt->next;
	// }
	// printf("\n");
	// tttt = ready_queue.head;
	// while(tttt){
	// 	printf("%s ",tttt->name);
	// 	tttt = tttt->next;
	// }

	if(task_pool.type == 0){ //FCFS
		if(!running) {
			running = ready_queue.head;
		}
	}
	else if(task_pool.type == 1){ //RR
		if(running){
			running->time_to_run--;
			if(running->time_to_run == 0){
				sprintf(running->another_me->state,"READY");
				if(ready_queue.size > 1){
					/* add it to ready_queue's tail */
					ready_queue.head = running->next;
					ready_queue.head->pre = NULL;
					ready_queue.tail->next = running;
					running->pre = ready_queue.tail;
					ready_queue.tail = running;
					ready_queue.tail->next = NULL;
				}
				else running->time_to_run = 3; //no other task in ready_queue, refresh it 
				
				running = ready_queue.head;
			}
		}
		else{
			running = ready_queue.head;
			if(running) running->time_to_run = 3; //time quantum is 30ms, with 10ms a unit
		}
	}
	else{ //PP
		int min_priority = 2147483647; //the lower the integer is, the higher priority the node has
		task_node *tmp = ready_queue.head;
		if(!tmp) return;
		
		task_node *next_to_run = NULL;
		while(tmp){
			if(tmp->priority < min_priority){
				next_to_run = tmp;
				min_priority = tmp->priority;
			}
			tmp = tmp->next;
		} 
	
		if(next_to_run != ready_queue.head){// move next_to_run to the first of ready_queue 
			if(next_to_run == ready_queue.tail){
				ready_queue.tail = next_to_run->pre;
				ready_queue.tail->next = NULL;
			}
			else{
				next_to_run->pre->next = next_to_run->next;
				next_to_run->next->pre = next_to_run->pre;
			}
			next_to_run->next = ready_queue.head;
			ready_queue.head->pre = next_to_run;
			ready_queue.head = next_to_run;
			ready_queue.head->pre = NULL;
		}
	
		running = next_to_run; //preemptive
	}
}

void signal_handler(){ //this is signal handler using alarm
	// if(ready_queue.tail) ready_queue.tail->next = NULL;
	// if(ready_queue.head) ready_queue.head->pre = NULL;
	// if(waiting_queue.tail) waiting_queue.tail->next = NULL;
	// if(waiting_queue.head) waiting_queue.head->pre = NULL;
	// printf("10ms passed!\n");
	// printf("%d %d\n",task_pool.size - task_pool.terminate_num,waiting_queue.size + ready_queue.size);
	if(ready_queue.size > 0){ // turnaround time and waiting time++ 
		if(ready_queue.tail == NULL) printf("error\n");
		if(ready_queue.head == NULL) printf("error\n");
		task_node *node = ready_queue.head->next;
		while(node){
			node->another_me->waiting_time++;
			node->turnaround_time++;
			node = node->next;
		}
	}

	if(running){
		running->another_me->running_time++;
		running->turnaround_time++;
	}

	task_node *node = waiting_queue.head;
	while(node){
		node->turnaround_time++;
		node = node->next;
	}

	task_node *original_running = running;

	rescheduling();

	if(original_running && original_running != running && strcmp(original_running->another_me->state,"TERMINATED") == 1) sprintf(original_running->another_me->state,"READY");
	if(!running){
		// if(waiting_queue.head) printf("%s ",waiting_queue.head->name);
		// for(int i = 0;i<8;i++)
		// 	printf("%d ",shared_resources[i]);
		printf("CPU idle.\n");
		
		return;
	}
	sprintf(running->another_me->state,"RUNNING");

	//printf("Task %s is running.\n",running->name);
	
	/* do context switch here */
	if(!original_running){
		printf("Task %s is running.\n",running->name); 
		setcontext(&running->ctx);
	}
	else if(running != original_running){
		printf("Task %s is running.\n",running->name); // only print if change?>
		swapcontext(&original_running->ctx,&running->ctx);
	}
	assert(ready_queue.size == 0 || ready_queue.head != NULL);
}

void ctrl_z(){ //detecting ctrl_z
	fst = 0;
	struct itimerval t1,t2;
	t1.it_interval.tv_usec = 0; ///10ms, can't be larger than 1 sec
    t1.it_interval.tv_sec = 0;
    t1.it_value.tv_usec = 0; //time distance to the next signal
    t1.it_value.tv_sec = 0;

	if(setitimer(ITIMER_VIRTUAL, &t1, &t2) < 0){ //stop tiemr
		perror("error in setting timer!\n");
		exit(0);
	}
	// ctrz = 1;
	// if(running) swapcontext(&running->ctx,&main_thread);
	// else setcontext(&main_thread);
	if(running) swapcontext(&running->ctx,&shell_ctx);
	else setcontext(&shell_ctx);
}

void scheduling(int type){
	ctrz = 0;
	/* timer setting */
    timer1.it_interval.tv_usec = 10000; ///10ms, can't be larger than 1 sec
    timer1.it_interval.tv_sec = 0;
    timer1.it_value.tv_usec = 10000; //time distance to the next signal
    timer1.it_value.tv_sec = 0;

    if( setitimer( ITIMER_VIRTUAL, &timer1, &timer2) < 0 ){ //use real time in system
        perror("error in setting timer!\n");
        return;
    }
    signal(SIGVTALRM, signal_handler); //time tick 
	/* timer setting end */

	signal(SIGTSTP,ctrl_z);

	getcontext(&main_thread);
    while(task_pool.terminate_num < task_pool.size){ //let program stop here before all tasks finished
		// if(ctrz){ //fake end
		// 	ctrz = 0;
		// 	return;
		// }
    }

	printf("Simulation over.\n");
	fst = 1;
	debug = 1;
	//return;
	setcontext(&shell_end);
}
