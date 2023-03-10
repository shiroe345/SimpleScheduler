#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/shell.h"
#include "include/command.h"
#include "include/task.h"
#include "include/resource.h"


void initial(int type){ //initialize task structure
    task_pool.head = NULL;
    task_pool.tail = NULL;
    task_pool.size = 0;
	task_pool.terminate_num = 0;
    task_pool.type = type;

	ready_queue.head = NULL;
	ready_queue.tail = NULL;
	ready_queue.size = 0;

	waiting_queue.head = NULL;
	waiting_queue.tail = NULL;
	waiting_queue.size = 0;

	task_pool.time_quantum = 3;//mean 30ms

	running = NULL;

	for(int i = 0;i<8;i++)
		shared_resources[i] = 0;
}

int main(int argc, char *argv[])
{
	fst = 1;
	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

	/* initialize task structure */
	if(argc != 2){
		printf("%d \n",argc);
		printf("Wrong input format\n");
		return -1;
	}
	else if(strcmp(argv[1],"FCFS") == 0)
		initial(0);
	else if(strcmp(argv[1],"RR") == 0)
		initial(1);
	else if(strcmp(argv[1],"PP") == 0)
		initial(2);
	else{
		printf("Wrong input arguments\n");
		return -1;
	}

	/* start the shell */
	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
