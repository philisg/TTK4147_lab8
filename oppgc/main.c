#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <pthread.h>

#define HIGH 80
#define MEDIUM 50
#define LOW 10

#define TIME_UNIT 100000

RT_MUTEX mutex;
RT_TASK task_l , task_m, task_h;

void busy_wait_us(unsigned long delay){
    for(; delay > 0; delay--){
        rt_timer_spin(TIME_UNIT);
    }
}

void low_function(void){
    rt_mutex_acquire(&mutex, TM_INFINITE);
    rt_printf("LOW THREAD HAS STARTED! \n");
    busy_wait_us(3);
    rt_printf("LOW THREAD HAS FINISHED! \n");
    rt_mutex_release(&mutex);

}

void medium_function(void){
    rt_task_sleep(TIME_UNIT * 1);
    rt_printf("MEDIUM THREAD HAS STARTED! \n");
    busy_wait_us(5);
    rt_printf("MEDIUM THREAD HAS FINISHED! \n");
}

void high_function(void){
    rt_task_sleep(TIME_UNIT * 2);
    rt_mutex_acquire(&mutex, TM_INFINITE);
    rt_printf("HIGH THREAD HAS STARTED! \n");
    busy_wait_us(2);
    rt_printf("HIGH THREAD HAS FINISHED! \n");
    rt_mutex_release(&mutex);

}

int main(){
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_task_shadow(NULL, NULL, 90, T_CPU(0));
	rt_print_auto_init(1);
	rt_mutex_create(&mutex, NULL);

	rt_task_create(&task_l, NULL, 0, LOW, T_CPU(0));
	rt_task_create(&task_m, NULL, 0, MEDIUM, T_CPU(0));
	rt_task_create(&task_h, NULL, 0, HIGH, T_CPU(0));

	rt_task_start(&task_l, (void*)low_function, NULL);
	rt_task_start(&task_m, (void*)medium_function, NULL);
	rt_task_start(&task_h, (void*)high_function, NULL);
	rt_task_sleep(100000000);
	//rt_sem_broadcast(&sem1);
	rt_task_sleep(100000000);

    sleep(5);
    rt_printf("Main finsihed\n");
	rt_mutex_delete(&mutex);
	return 0;
}