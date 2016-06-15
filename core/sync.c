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
	// timeout 처리를 위해 system timer 를 가져온다
	eos_counter_t *sys_timer = eos_get_system_timer();
	// timeout 시간에 현재 system timer ticker 을 더해서 timeout 이 발생할 tick 을 구한다.
	int32u_t timeout_tick = timeout + sys_timer->tick;
	int32u_t flag;

	while (1) {
		// timeout 됬는지 여부를 boolean 으로 저장한다.
		bool_t timeouted = (timeout <= 0) && (timeout_tick < sys_timer->tick);
		// semaphore 를 획득할 때는 다른 interrupt 가 못껴들게 disable 한다.
		flag = eos_disable_interrupt();

		if (sem->count > 0) {
			// count > 0 이면, count를 1 감소시키고 리턴 (성공)
			(sem->count)--;
			eos_restore_interrupt(flag);
			// if (!timeouted) {
			// (sem->count)--;
			// eos_restore_interrupt(flag);
			// } else if (sem->wait_queue) {
			// 	// 현재 태스크가 timeout 됬고 wating_queue 에 task 가 있으면 
			// 	// waiting_queue 에서 대기중인 task 를 깨운다.
			// 	_os_wakeup_single(&(sem->wait_queue), sem->queue_type);
			// 	eos_restore_interrupt(flag);
			// 	eos_schedule();
			// }
			return 1;
		}

		bool_t should_not_wait = (timeout == -1) || timeouted;
		if (should_not_wait) {
			eos_restore_interrupt(flag);
			return 0;
		}

		// put the task into waiting queue if it should wait
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
		eos_restore_interrupt(flag);
		eos_schedule();
	}
}

void eos_release_semaphore(eos_semaphore_t *sem) {
	// semaphore 를 release 할 때 다른 작업이 못껴들게 interrupt disable
	int32u_t flag = eos_disable_interrupt();
	// semaphore count 1 증가
	(sem->count)++;
	// wait 큐에 대기중인 태스크가 있는 경우 하나의 태스크를 선택해 깨움
	if (sem->wait_queue) {
		_os_wakeup_single(&(sem->wait_queue), sem->queue_type);
	}
	// interrupt disable 을 되돌린다.
	eos_restore_interrupt(flag);
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
