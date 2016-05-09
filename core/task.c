/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Assignment 2 Task Pointer
 */
static eos_tcb_t *idle_task;
static eos_tcb_t *number_task;
static eos_tcb_t *alphabet_task;

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

    task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);
    task->priority = priority;
    task->status = READY;

    if (idle_task == NULL) {
    	idle_task = task;
    } else if (number_task == NULL) {
    	number_task = task;
    } else if (alphabet_task == NULL) {
    	number_task = task;
    }
    return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
	if (_os_current_task != NULL) {
		addr_t saved_sp = _os_save_context();

		if (saved_sp == 0) {
			return;
		} else {
			_os_current_task->sp = saved_sp;
		}
	}

	if (_os_current_task == NULL) {
		_os_current_task = number_task;
	} else if (_os_current_task == number_task) {
		_os_current_task = alphabet_task;
	} else if (_os_current_task == alphabet_task) {
		_os_current_task = number_task;
	}

	 _os_restore_context(_os_current_task->sp);
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
}
