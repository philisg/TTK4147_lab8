#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <rtdk.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <native/sem.h>

RT_TASK taskresponse_thread_1;
RT_TASK taskresponse_thread_2;
RT_TASK taskresponse_thread_main;
RT_SEM sem1, sem2;
int set_cpu(int cpu_number);


void responseTask1(void) {
	set_cpu(T_CPU(0));
    rt_sem_p(&sem1, TM_INFINITE);
    rt_timer_spin(100*1000*1000); //100ms
    rt_printf("Task1\n");
    return 0;
}

void responseTask2(void) {
	set_cpu(T_CPU(0));
    rt_sem_p(&sem1, TM_INFINITE);
    rt_timer_spin(100*1000*1000); //100ms
    rt_printf("Task2\n");
    return 0;
}


int main (void){
    mlockall(MCL_CURRENT|MCL_FUTURE);
    rt_print_auto_init(1);

    rt_printf("The program has started!\n");
    rt_sem_create(&sem1,"Semaphore1", 0, S_FIFO);
    //rt_sem_create(&sem2,"Semaphore2", 0, S_FIFO);

    
    rt_task_shadow(NULL, "main" ,99, T_CPU(0));
    rt_task_start(&taskresponse_thread_main, NULL, NULL); //start the task

    rt_task_create(&taskresponse_thread_1, "task1", 0, 55, T_CPU(0));//creating task, 50=priority
    rt_task_create(&taskresponse_thread_2, "task2", 0, 50, T_CPU(0));//creating task
        
    rt_task_start(&taskresponse_thread_1, &responseTask1, NULL); //start the task
    rt_task_start(&taskresponse_thread_2, &responseTask2, NULL); //start the task

    rt_timer_spin(500*1000*1000); //100ms

    rt_sem_broadcast(&sem1);
    //rt_sem_broadcast(&sem2);


    //broadcast sem!
    rt_sem_delete(&sem1);

    return 0;
    
}

int set_cpu(int cpu_number) {
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}