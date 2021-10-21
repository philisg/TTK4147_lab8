 //#define _GNU_SOURCE
#include <rtdk.h>
#include <pthread.h>
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <native/mutex.h>

#include <sys/mman.h>
#include <unistd.h>

RT_SEM sem;
RT_MUTEX mutex_a;
RT_MUTEX mutex_b;

typedef struct TaskParameters{
    uint8_t id;
    uint8_t priority;
}TaskParameters;

int set_cpu(int cpu_number){
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);

	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

void busy_wait_us(unsigned long delay){
    for(; delay > 0; delay--){
        rt_timer_spin(10000);
    }
}

void task_func(void * args){
    set_cpu(1);
    struct TaskParameters taskparam = *(struct TaskParameters*)args;
    // rt_printf("Task ID: %d\tPriority: %d\r\n", taskparam.id, taskparam.priority);
    switch(taskparam.id){
        case 1: 
            rt_sem_p(&sem, TM_INFINITE);
            rt_mutex_acquire(&mutex_a, TM_INFINITE);
            rt_printf("Task: %d\t...Acquire Mutex A\r\n", taskparam.id);
            rt_printf("Task: %d\t...Busy-wait for 3 time units\r\n", taskparam.id);
            busy_wait_us(3);
            rt_printf("Task: %d\t...Acquire Mutex B\r\n", taskparam.id);
            rt_mutex_acquire(&mutex_b, TM_INFINITE);
            rt_printf("Task: %d\t...Busy-wait for 3 time units\r\n", taskparam.id);
            busy_wait_us(3);
            rt_printf("Task: %d\t...Release Mutex B\r\n", taskparam.id);
            rt_mutex_release(&mutex_b);
            rt_printf("Task: %d\t...Release Mutex A\r\n", taskparam.id);
            rt_mutex_release(&mutex_a);
            rt_printf("Task: %d\t...Busy-wait for 1 time unit\r\n", taskparam.id);
            busy_wait_us(1);
            // rt_sem_v(&sem);
            break;
        case 2:
            rt_sem_p(&sem, TM_INFINITE);
            // rt_printf("Task: %d\t...Sleep for 1 time unit\r\n", taskparam.id);
            rt_task_sleep(10000);
            rt_mutex_acquire(&mutex_b, TM_INFINITE);
            rt_printf("Task: %d\t...Acquire Mutex B\r\n", taskparam.id);
            rt_printf("Task: %d\t...Busy-wait for 1 time unit\r\n", taskparam.id);
            busy_wait_us(1);
            rt_printf("Task: %d\t...Acquire Mutex A\r\n", taskparam.id);
            rt_mutex_acquire(&mutex_a, TM_INFINITE);
            rt_printf("Task: %d\t...Busy-wait for 2 time units\r\n", taskparam.id);
            busy_wait_us(2);
            rt_printf("Task: %d\t...Release Mutex A\r\n", taskparam.id);
            rt_mutex_release(&mutex_a);
            rt_printf("Task: %d\t...Release Mutex B\r\n", taskparam.id);
            rt_mutex_release(&mutex_b);
            rt_printf("Task: %d\t...Busy-wait for 1 time unit\r\n", taskparam.id);
            busy_wait_us(1);
            // rt_sem_v(&sem);
            break;
    }
}


int main(){
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_print_auto_init(1);
	int stack_size = 0;
	int mode = T_CPU(1);

    int task1_id = 1;
    int task2_id = 2;

    int task1_prio = 60;
    int task2_prio = 70;

    rt_task_shadow(NULL, "Main", 75, mode);

    rt_sem_create(&sem, "Semaphore", 0, S_PRIO);

    RT_TASK task1;
    RT_TASK task2;

    rt_mutex_create(&mutex_a,"A Mutex");
    rt_mutex_create(&mutex_b,"B Mutex");
 
	//Creating threads
	rt_task_create(&task1, "Task1", stack_size, task1_prio, mode);
	rt_task_create(&task2, "Task2", stack_size, task2_prio, mode);

	//Start threads
	rt_task_start(&task1, task_func,(&(struct TaskParameters){task1_id, task1_prio}));
	rt_task_start(&task2, task_func,(&(struct TaskParameters){task2_id, task2_prio}));

    rt_task_sleep(100*1000*1000);
    rt_printf("Broadcasting...\r\n");
    rt_sem_broadcast(&sem);
    rt_task_sleep(100*1000*1000);
    rt_printf("END...\r\n");
    sleep(5);
    rt_sem_delete(&sem);
    rt_mutex_delete(&mutex_a);
    rt_mutex_delete(&mutex_b);



    return 0;
}