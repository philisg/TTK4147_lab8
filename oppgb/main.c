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

#define TIME_UNIT 10000

RT_SEM sem1;
RT_TASK task_l , task_m, task_h;
int set_cpu(int cpu_number);

void busy_wait_us(unsigned long delay){
		for(; delay > 0; delay--){
			rt_timer_spin(TIME_UNIT);
		}
}

void low_function(void){
    set_cpu(T_CPU(0));
    rt_sem_p(&sem1, TM_INFINITE);
    rt_printf("LOW THREAD HAS STARTED! \n");
    busy_wait_us(3);
    rt_sem_v(&sem1);
    rt_printf("LOW THREAD HAS RUN! \n");
}

void medium_function(void){
    set_cpu(T_CPU(0));
    rt_task_sleep(TIME_UNIT * 1);
    rt_printf("MEDIUM THREAD HAS STARTED! \n");
    busy_wait_us(5);
    rt_printf("MEDIUM THREAD HAS RUN! \n");
}

void high_function(void){
    set_cpu(T_CPU(0));
    rt_task_sleep(TIME_UNIT * 2);
    rt_sem_p(&sem1, TM_INFINITE);
    rt_printf("HIGH THREAD HAS STARTED! \n");
    busy_wait_us(2);
    rt_sem_v(&sem1);
    rt_printf("HIGH THREAD HAS RUN! \n");
}

int main(){
	mlockall(MCL_CURRENT | MCL_FUTURE);
    rt_print_auto_init(1);
	rt_task_shadow(NULL, NULL, 90, T_CPU(0));

	rt_sem_create(&sem1, NULL, 1, NULL);

	rt_task_create(&task_l, NULL, 0, 60, T_CPU(0));
	rt_task_create(&task_m, NULL, 0, 70, T_CPU(0));
	rt_task_create(&task_h, NULL, 0, 80, T_CPU(0));

	rt_task_start(&task_l, (void*)low_function, NULL);
	rt_task_start(&task_m, (void*)medium_function, NULL);
	rt_task_start(&task_h, (void*)high_function, NULL);

	rt_task_sleep(100000000);
	//rt_sem_broadcast(&sem1);
	rt_task_sleep(100000000);

	rt_sem_delete(&sem1);
	return 0;
}

int set_cpu(int cpu_number) {
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);
	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}
