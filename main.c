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

RT_TASK HIGH_thread;
RT_TASK MEDIUM_thread;
RT_TASK LOW_thread;
RT_SEM sem1;
int set_cpu(int cpu_number);

void busy_wait_us(unsigned long delay){
    for(; delay > 0; delay--){
        rt_timer_spin(1000);
    }
}
void HIGH(void) {
	set_cpu(T_CPU(0));
    rt_task_sleep(2000);
    rt_sem_p(&sem1, TM_INFINITE);
    busy_wait_us(2);
    rt_sem_v(&sem1);
}

void MEDIUM(void) {
	set_cpu(T_CPU(0));
    rt_task_sleep(1000);
    busy_wait_us(5);
    rt_printf("Tas MEDIUM!\n");

}

void LOW(void) {
	set_cpu(T_CPU(0));
    rt_sem_p(&sem1, TM_INFINITE);
    rt_printf("LOW\n");
    busy_wait_us(3);
    rt_sem_v(&sem1);

}


int main (void){
    mlockall(MCL_CURRENT|MCL_FUTURE);
    rt_task_shadow(NULL, NULL ,90, T_CPU(0));
    rt_print_auto_init(1);

    rt_printf("The program has started!\n");
    rt_sem_create(&sem1,"Semaphore1", 1, S_FIFO);
    //rt_sem_create(&sem2,"Semaphore2", 0, S_FIFO);


    rt_task_create(&HIGH_thread, "task1", 0, 55, T_CPU(0));//creating task, 50=priority
    rt_task_create(&MEDIUM_thread, "task2", 0, 50, T_CPU(0));//creating task
    rt_task_create(&LOW_thread, "task3", 0, 40, T_CPU(0));//creating task  rt_task_create(&MEDIUM, "task2", 0, 50, T_CPU(0));//creating task

    rt_task_start(&LOW, &LOW_thread, NULL); //start the task
    rt_task_start(&MEDIUM, &MEDIUM_thread, NULL); //start the task
    rt_task_start(&HIGH, &HIGH_thread, NULL); //start the task_

    rt_task_sleep(500000000); //500ms

    //rt_sem_broadcast(&sem1);
    //rt_sem_broadcast(&sem2);
    rt_task_sleep(100000000); //100ms

    rt_sem_delete(&sem1);

    return 0;
    
}

int set_cpu(int cpu_number) {
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}