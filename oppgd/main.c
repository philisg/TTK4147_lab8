#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <pthread.h>

#define HIGH 80
#define MEDIUM 50
#define LOW 10

#define TIME_UNIT 100

RT_MUTEX mutexA, mutexB;
RT_TASK task_l , task_m, task_h;
RT_SEM sem;

void busy_wait_us(unsigned long delay){
    for(; delay > 0; delay--){
        rt_timer_spin(1000);
    }
}

void low_function(void){
    rt_sem_p(&sem, TM_INFINITE);

    rt_mutex_acquire(&mutexA, TM_INFINITE);

    rt_printf("LOW THREAD HAS STARTED! \n");
    busy_wait_us(3*TIME_UNIT);

    rt_mutex_acquire(&mutexB, TM_INFINITE);
    busy_wait_us(3*TIME_UNIT);

    rt_mutex_release(&mutexB);
    rt_mutex_release(&mutexA);

    busy_wait_us(1*TIME_UNIT);
    rt_printf("LOW THREAD HAS FINISHED! \n");
}

void medium_function(void){
    rt_task_sleep(TIME_UNIT * 1000);
    rt_printf("MEDIUM THREAD HAS STARTED! \n");
    busy_wait_us(5*TIME_UNIT);
    rt_printf("MEDIUM THREAD HAS FINISHED! \n");
}

void high_function(void){
    rt_sem_p(&sem, TM_INFINITE);

    rt_task_sleep(TIME_UNIT * 1);
    rt_mutex_acquire(&mutexB, TM_INFINITE);
    rt_printf("HIGH THREAD HAS STARTED! \n");
    busy_wait_us(1*TIME_UNIT);
    rt_mutex_acquire(&mutexA, TM_INFINITE);
    busy_wait_us(2*TIME_UNIT);
    rt_mutex_release(&mutexA);
    rt_mutex_release(&mutexB);
    busy_wait_us(1*TIME_UNIT);
    rt_printf("HIGH THREAD HAS FINISHED! \n");
}

int main(){
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_task_shadow(NULL, NULL, 90, T_CPU(0));
	rt_print_auto_init(1);
	rt_mutex_create(&mutexA, NULL);
    rt_mutex_create(&mutexB, NULL);
    rt_sem_create(&sem,"Semaphore", 1, S_FIFO);

	rt_task_create(&task_l, "task_low", 0, 40, T_CPU(0));
	//rt_task_create(&task_m, NULL, 0, MEDIUM, T_CPU(0));
	rt_task_create(&task_h, "task_high", 0, 50, T_CPU(0));

	rt_task_start(&task_l, (void*)low_function, NULL);
	//rt_task_start(&task_m, (void*)medium_function, NULL);
	rt_task_start(&task_h, (void*)high_function, NULL);

	rt_task_sleep(100000000);
	rt_sem_broadcast(&sem);
	rt_task_sleep(100000000);

    while(1);

	rt_mutex_delete(&mutexA);
    rt_mutex_delete(&mutexB);
     rt_sem_delete(&sem);
	return 0;
}