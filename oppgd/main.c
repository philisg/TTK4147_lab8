#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <native/sem.h>

#define TIME_UNIT 100

RT_MUTEX mutexA, mutexB;
RT_TASK task_l , task_h;
RT_SEM sem;
RT_MUTEX_INFO info_A, info_B;

int set_cpu(int cpu_number);

void busy_wait_us(unsigned long delay){
    for(; delay > 0; delay--){
        rt_timer_spin(1000);
    }
}

void low_function(void){
    set_cpu(T_CPU(0));

    rt_printf("LOW THREAD READY \n");
    rt_sem_p(&sem, TM_INFINITE);

    rt_mutex_acquire(&mutexA, TM_INFINITE);
    rt_printf("l-func takes mutexA \n");

    busy_wait_us(3*TIME_UNIT);
    rt_printf("l-func done busy waiting \n");

    rt_mutex_acquire(&mutexB, TM_INFINITE);
    rt_printf("l-func takes mutexB \n");

    busy_wait_us(3*TIME_UNIT);
    rt_printf("l-func done busy waiting \n");

    rt_mutex_release(&mutexB);
    rt_mutex_release(&mutexA);
    rt_printf("l-func released mutexes \n");

    busy_wait_us(1*TIME_UNIT);
    rt_printf("LOW THREAD HAS FINISHED! \n");
}

void high_function(void){
    set_cpu(T_CPU(0));

    rt_printf("HIGH THREAD READY\n");
    rt_sem_p(&sem, TM_INFINITE);

    rt_task_sleep(TIME_UNIT * 1);
    rt_printf("h-func done sleeping \n");

    rt_mutex_acquire(&mutexB, TM_INFINITE);
    rt_printf("h-func takes mutexB \n");

    busy_wait_us(1*TIME_UNIT);
    rt_printf("h-func done busy waiting \n");

    rt_mutex_acquire(&mutexA, TM_INFINITE);
    rt_printf("h-func takes mutexA \n");

    busy_wait_us(2*TIME_UNIT);
    rt_printf("h-func done busy waiting \n");

    rt_mutex_release(&mutexA);
    rt_mutex_release(&mutexB);
    rt_printf("h-func released mutexes \n");

    busy_wait_us(1*TIME_UNIT);
    rt_printf("HIGH THREAD HAS FINISHED! \n");
}

int main(){
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_task_shadow(NULL, NULL, 90, T_CPU(0));
	rt_print_auto_init(1);

	rt_mutex_create(&mutexA, NULL);
    rt_mutex_create(&mutexB, NULL);
    rt_sem_create(&sem,"Semaphore", 0, S_FIFO);

	rt_task_create(&task_l, "task_low", 0, 70, T_CPU(0));
	rt_task_create(&task_h, "task_high", 0, 80, T_CPU(0));

	rt_task_start(&task_l, (void*)low_function, NULL);
	rt_task_start(&task_h, (void*)high_function, NULL);

	rt_task_sleep(100*1000*1000); //100ms
	rt_sem_broadcast(&sem);
	rt_task_sleep(100*1000*1000); //100ms

    while(1);

	rt_mutex_delete(&mutexA);
    rt_mutex_delete(&mutexB);
    rt_sem_delete(&sem);
	return 0;
}

int set_cpu(int cpu_number) {
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}