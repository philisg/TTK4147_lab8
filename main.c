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
#include <native/mutex.h>

RT_TASK HIGH_thread;
RT_TASK MEDIUM_thread;
RT_TASK LOW_thread;

RT_MUTEX mutex;
#define TIME_UNIT 100

RT_SEM sem1, sem2;
int set_cpu(int cpu_number);

void busy_wait_us(unsigned long delay){
    for(; delay > 0; delay--){
        rt_timer_spin(1000);
    }
}
void HIGH(void) {
	set_cpu(T_CPU(0));

    rt_sem_p(&sem2, TM_INFINITE);

    rt_task_sleep(2000);
    //rt_sem_p(&sem1, TM_INFINITE);
    rt_mutex_aquire($mutex, TM_INFINITE);

    rt_printf("Task HIGH starts busy work\n");
    busy_wait_us(2*TIME_UNIT);
    rt_printf("Task HIGH ends busy work\n");

    //rt_sem_v(&sem1);
    rt_mutex_release(&mutex);
}

void MEDIUM(void) {
	set_cpu(T_CPU(0));

    rt_sem_p(&sem2, TM_INFINITE);

    rt_task_sleep(1000);
    rt_printf("Task MED starts busy work\n");
    busy_wait_us(5*TIME_UNIT);
    rt_printf("Task MED ends busy work!\n");
}

void LOW(void) {
	set_cpu(T_CPU(0));

    rt_sem_p(&sem2, TM_INFINITE);

    //rt_sem_p(&sem1, TM_INFINITE);
    rt_mutex_acquire(&mutex, TM_INFINITE);

    rt_printf("Task LOW starts busy work\n");
    busy_wait_us(1000*TIME_UNIT);
    rt_printf("Task LOW ends busy work\n");

    //rt_sem_v(&sem1);
    rt_mutex_release(&mutex);
}


int main (void){
    mlockall(MCL_CURRENT|MCL_FUTURE);
    rt_task_shadow(NULL, NULL ,90, T_CPU(0));
    rt_print_auto_init(1);

    rt_printf("The program has started!\n");
    rt_sem_create(&sem1,"Semaphore1", 1, S_FIFO);
    rt_sem_create(&sem2, "Semaphore2", 0, S_FIFO);

    rt_mutex_create(&mutex, NULL);

    rt_task_create(&HIGH_thread, "task-high", 0, 55, T_CPU(0));//creating task, 50=priority
    rt_task_create(&MEDIUM_thread, "task-medium", 0, 50, T_CPU(0));//creating task
    rt_task_create(&LOW_thread, "task-low", 0, 40, T_CPU(0)); //creating task  rt_task_create(&MEDIUM, "task2", 0, 50, T_CPU(0));//creating task

    rt_task_start(&LOW_thread, &LOW, NULL); //start the task
    rt_task_start(&MEDIUM_thread, &MEDIUM, NULL); //start the task
    rt_task_start(&HIGH_thread, &HIGH, NULL); //start the task_

    rt_task_sleep(500*1000*1000); //500ms

    //rt_sem_broadcast(&sem1);
    rt_sem_broadcast(&sem2);
    rt_task_sleep(1000*1000*1000); //100ms

    rt_sem_delete(&sem1);
    rt_sem_delete(&sem2);
    rt_mutex_delete(&mutex);

    return 0;
    
}

int set_cpu(int cpu_number) {
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}