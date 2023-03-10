#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/task.h"
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/scheduler.h"
#include "../include/function.h"
#include "../include/resource.h"

const char *functions[] = {
 	"test_exit",
 	"test_sleep",
	"test_resource1",
 	"test_resource2",
 	"idle",
	"task1",
	"task2",
	"task3",
	"task4",
	"task5",
    "task6",
    "task7",
    "task8",
    "task9",
};

const void (*func[])(void) = {
	&test_exit,
 	&test_sleep,
	&test_resource1,
 	&test_resource2,
 	&idle,
	&task1,
	&task2,
	&task3,
	&task4,
	&task5,
    &task6,
    &task7,
    &task8,
    &task9
};

int num_functions() {
	return sizeof(functions) / sizeof(char *);
}


int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
	printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    	if (chdir(args[1]) != 0)
      		perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i) {
    	if(str[i] >= 48 && str[i] <= 57)
			continue;
        else
		    return false;
  	}
  	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if(!args[1]){ //no arguments , TA's code no this!(if enter mypid , it will error)
		printf("wrong type! Please type again!\n");
	}
	else if(strcmp(args[1], "-i") == 0) {

	    pid_t pid = getpid();
	    printf("%d\n", pid);
	
	} else if (strcmp(args[1], "-p") == 0) {
	
		if (args[2] == NULL) {
      		printf("mypid -p: too few argument\n");
      		return 1;
    	}

    	sprintf(fname, "/proc/%s/stat", args[2]);
    	int fd = open(fname, O_RDONLY);
    	if(fd == -1) {
      		printf("mypid -p: process id not exist\n");
     		return 1;
    	}

    	read(fd, buffer, BUF_SIZE);
	    strtok(buffer, " ");
    	strtok(NULL, " ");
	    strtok(NULL, " ");
    	char *s_ppid = strtok(NULL, " ");
	    int ppid = strtol(s_ppid, NULL, 10);
    	printf("%d\n", ppid);
	    
		close(fd);

  	} else if (strcmp(args[1], "-c") == 0) {

		if (args[2] == NULL) {
      		printf("mypid -c: too few argument\n");
      		return 1;
    	}

    	DIR *dirp;
    	if ((dirp = opendir("/proc/")) == NULL){
      		printf("open directory error!\n");
      		return 1;
    	}

    	struct dirent *direntp;
    	while ((direntp = readdir(dirp)) != NULL) {
      		if (!isnum(direntp->d_name)) {
        		continue;
      		} else {
        		sprintf(fname, "/proc/%s/stat", direntp->d_name);
		        int fd = open(fname, O_RDONLY);
        		if (fd == -1) {
          			printf("mypid -p: process id not exist\n");
          			return 1;
        		}

        		read(fd, buffer, BUF_SIZE);
        		strtok(buffer, " ");
        		strtok(NULL, " ");
        		strtok(NULL, " ");
		        char *s_ppid = strtok(NULL, " ");
		        if(strcmp(s_ppid, args[2]) == 0)
		            printf("%s\n", direntp->d_name);

        		close(fd);
     		}
	   	}
    	
		closedir(dirp);
	
	} else {
    	printf("wrong type! Please type again!\n");
  	}
	
	return 1;
}

int add(char **args)
{
	task_pool.size++;
	ready_queue.size++;

	/* initialize a new task node to a task_pool*/
	task_node *new = (task_node*)malloc(sizeof(task_node));
	strcpy(new->name, args[1]);
	strcpy(new->func,args[2]);
	//getcontext(&new->ctx);
	new->number_of_resource = 0;
	new->priority = atoi(args[3]);
	sprintf(new->resource,"none");
	new->tid = task_pool.size;
	new->running_time = 0;
	sprintf(new->state,"READY");
	new->turnaround_time = 0;
	new->waiting_time = 0;
	new->next = NULL;
	new->pre = NULL;

	if(!task_pool.head){
		task_pool.head = new;
		task_pool.tail = new;
	}
	else{
		task_pool.tail->next = new;
		new->pre = task_pool.tail;
		task_pool.tail = new;
	}

	/* initialize a new task node to a ready_queue */
	task_node *new2 = (task_node*)malloc(sizeof(task_node));
	strcpy(new2->name, args[1]);
	strcpy(new2->func,args[2]);
	new2->number_of_resource = 0;
	new2->priority = atoi(args[3]);
	sprintf(new2->resource,"none");
	new2->tid = task_pool.size;
	new2->running_time = 0;
	sprintf(new2->state,"READY");
	new2->turnaround_time = 0;
	new2->waiting_time = 0;
	new2->next = NULL;
	new2->pre = NULL;
	new2->another_me = new; //point to information node in task_pool (easier to change information, ex: running_time, waiting_time, state)

	/* ucontext initialization */
	getcontext(&new2->ctx);
	new2->ctx.uc_stack.ss_sp = new2->stk;//指定棧空間
    new2->ctx.uc_stack.ss_size = sizeof(new2->stk);//指定棧空間大小
    new2->ctx.uc_stack.ss_flags = 0;
    new2->ctx.uc_link = &main_thread;
	for (int i = 0; i < num_functions(); i++)
		if (strcmp(new2->func, functions[i]) == 0)
			makecontext(&new2->ctx,func[i],0);

	if(!ready_queue.head){
		ready_queue.head = new2;
		ready_queue.tail = new2;
		ready_queue.cur = new2;
	}
	else{
		ready_queue.tail->next = new2;
		new2->pre = ready_queue.tail;
		ready_queue.tail = new2;
	}

	printf("Task %s is ready.\n",new->name);
	return 1;
}

int del(char **args)
{
	if(!args[1]){
		printf("NO task name input!!!\n");
		return 1;
	}

	int c = 0; //1 is in ready_queue, 2 is in waiting queue;
	// if(running) printf("%s\n",running->another_me->name);
	task_node* tmp = task_pool.head;
	while(tmp){
		if(strcmp(tmp->name,args[1]) == 0){
			if(strcmp(tmp->state,"READY") == 0 || strcmp(tmp->state,"RUNNING") == 0) c = 1;
			else if(strcmp(tmp->state,"WAITING") == 0) c = 2;
			sprintf(tmp->state,"TERMINATED");
			printf("Task %s is killed.\n",tmp->name);
			break;
		}
		tmp = tmp -> next;
	}
	if(!tmp){
		printf("No this task is found!!!\n");
		return 1;
	}
	int turnaround_time = 0;
	
	// printf("%s %s %d\n",tmp->name,tmp->state,c);
	if(c == 1){ //the task is in ready queue
		task_node* t = ready_queue.head;
		ready_queue.size--;
		while(t){
			if(strcmp(t->name,args[1]) == 0){
				turnaround_time = t->turnaround_time;

				if(t == ready_queue.cur)
					ready_queue.cur = t->next;//should b next
			
				if(t == ready_queue.head)
					ready_queue.head = t->next;
				if(t == ready_queue.tail)
					ready_queue.tail = t->pre;
				if(t->pre) t->pre->next = t->next;
				if(t->next) t->next->pre = t->pre;

				if(ready_queue.head) ready_queue.head->pre = NULL;
				if(ready_queue.tail) ready_queue.tail->next = NULL;

	
				if(t == running){ //check if del running
					for(int i = 0;i<8;i++){ //check resource
						if(shared_resources[i] == t->tid)
							shared_resources[i] = 0; //release
					}
					sprintf(t->another_me->resource,"none");
					running = NULL;
					// printf("c = 1\n");
				}
				t = NULL;
				break;;

			}
			t = t->next;
		}
	}
	else if(c == 2){//the task is in waiting queue
		task_node* t = waiting_queue.head;
		waiting_queue.size--;

		while(t){
			if(strcmp(t->name,args[1]) == 0){
				
				turnaround_time = t->turnaround_time;
				
				if(t == waiting_queue.head)
					waiting_queue.head = t->next;
				if(t == waiting_queue.tail)
					waiting_queue.tail = t->pre;
				if(t->pre) t->pre->next = t->next;
				if(t->next) t->next->pre = t->pre;

				if(waiting_queue.head) waiting_queue.head->pre = NULL;
				if(waiting_queue.tail) waiting_queue.tail->next = NULL;

				for(int i = 0;i<8;i++){ //check resource
					if(shared_resources[i] == t->tid)
						shared_resources[i] = 0; //release
				}
				sprintf(t->another_me->resource,"none");
				t = NULL;
				break;
			}
			t = t->next;
		}
	}
	
	tmp->turnaround_time = turnaround_time; //this line true?

	task_pool.terminate_num++;
	return 1;
}

int ps(char **args)
{
	printf(" TID|       name|      state| running| waiting| turnaround|       resources| priority\n");
	printf("-------------------------------------------------------------------------------------\n");
	task_node* tmp = task_pool.head;
	while(tmp){
		char t[20];
		if(tmp->turnaround_time == 0) sprintf(t,"none");
		else sprintf(t,"%11d",tmp->turnaround_time);
		printf("%4d|%11s|%11s|%8d|%8d|%11s|%16s|%9d\n",tmp->tid,tmp->name,tmp->state,tmp->running_time,tmp->waiting_time,t,tmp->resource,tmp->priority);
		tmp = tmp->next;
	}
	return 1;
}

int start(char **args)
{
	printf("Start simulation.\n");
	if(fst) scheduling();
	else{
		if( setitimer( ITIMER_VIRTUAL, &timer1, &timer2) < 0 ){ //use real time in system
			perror("error in setting timer!\n");
			return 0;
		}
		if(running != NULL){
			// printf("running\n");
			// setcontext(&running->ctx);
			swapcontext(&shell_ctx, &running->ctx);
		}
		// else setcontext(&main_thread);
		else swapcontext(&shell_ctx, &main_thread);
	}
	return 1;
}

const char *builtin_str[] = {
 	"help",
 	"cd",
	"echo",
 	"exit",
 	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start"
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
  	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
