#include <ucontext.h>
#include <sys/time.h>


#ifndef TASK_H
#define TASK_H

void task_sleep(int ten_ms);
void task_exit();
int cts();
int cts2();

typedef struct task_node
{
    ucontext_t ctx;
    char name[1000];
    char func[1000]; //do which function
    char stk[1024 * 128]; //used to be ss_sp
    int sleeping; //determine sleep or wait resource
    int priority;
    char state[10]; //running, ready, waiting or terminated
    int running_time;
    int turnaround_time;
    int waiting_time;// already waited time
    int time_to_wait;// left to wait
    int time_to_run;// left to run
    int tid; //from 1, is unique
    int number_of_resource;
    int resources[8];
    char resource[20]; //used to print
    struct task_node *next, *pre;
    struct task_node* another_me; //point to information node in task_pool
}task_node;

typedef struct tasks //the data sturcture to record task
{
    task_node *head, *tail;
    int size;
    int type; //fcfs(0), rr(1) or pp(2)
    int time_quantum; //needed if rr, 10ms a unit
    int terminate_num; //the number of terminated procecess
}tasks;

typedef struct task_queue
{
    task_node *head, *tail;
    task_node *cur; //the next should be executed in ready q
    int size;
}task_queue;


task_node* running; //the task that is running in ready_queue
tasks task_pool; //store all the tasks
task_queue ready_queue;
task_queue waiting_queue;
ucontext_t main_thread;
ucontext_t shell_ctx, shell_end;
int fst; //first starts
int ctrz; //ctrz
int debug;
struct itimerval timer1, timer2;

#endif
