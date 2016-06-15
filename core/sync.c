/********************************************************
 * Filename: core/sync.c
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	/* initialization */
	sem->count = initial_count;
	sem->wait_queue = NULL;
	sem->queue_type = queue_type;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
	eos_counter_t *sys_timer = eos_get_system_timer();
	int32u_t timeout_tick = (timeout <= 0) ? 0 : timeout + sys_timer->tick;   // not 0 if there is an timeout
	int32u_t prev_eflags;

	/* waiting semaphore loop */
	while (1) {
		bool_t timeouted = (timeout_tick != 0) && (sys_timer->tick > timeout_tick);
		prev_eflags= eos_disable_interrupt();

		if (sem->count > 0) {
			/* given semaphore is now available */
			if (!timeouted) {
				(sem->count)--;
				eos_restore_interrupt(prev_eflags);
			} else if (sem->wait_queue) {
				// if this task timeouted, wakeup another task in waiting queue
				_os_wakeup_single(&(sem->wait_queue), sem->queue_type);
				eos_restore_interrupt(prev_eflags);
				eos_schedule();
			}
			return timeouted ? 0 : 1;
		}

		/* given semaphore is not available now */
		bool_t should_not_wait = (timeout < 0) || timeouted;
		if (should_not_wait) {
			eos_restore_interrupt(prev_eflags);
			return 0;
		}

		// PRINT("4\n");
		// put the task into waiting queue if it should wait
		int32u_t WAITING = 3;
		eos_tcb_t *tsk = eos_get_current_task();
		tsk->status = WAITING;
		if (sem->queue_type == FIFO) {
    		_os_add_node_tail(&(sem->wait_queue), &(tsk->node));
		} else if (sem->queue_type == PRIORITY) {
    		_os_add_node_priority(&(sem->wait_queue), &(tsk->node));
		} else {
			printf("Unexpected semaphore queue type.\n");
			return -1;
		}
		eos_restore_interrupt(prev_eflags);
		eos_schedule();
	}
}

void eos_release_semaphore(eos_semaphore_t *sem) {
	int32u_t prev_eflags;

	prev_eflags = eos_disable_interrupt();
	(sem->count)++;
	if (sem->wait_queue) _os_wakeup_single(&(sem->wait_queue), sem->queue_type);
	eos_restore_interrupt(prev_eflags);
	// PRINT("rs im %d, %d\n", eos_get_current_task()->period, eos_get_current_task()->status);
	eos_schedule();
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
