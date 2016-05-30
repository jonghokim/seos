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
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

    task->priority = priority;
    task->status = READY;
    task->node.ptr_data = task;
    task->node.priority = 0;  
    task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);

	_os_add_node_tail(_os_ready_queue + priority, &(task->node));
	_os_set_ready(priority);

    return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
    PRINT("eos_schedule start.\n");
	if (_os_current_task != NULL && _os_current_task->status == RUNNING) {
        PRINT("eos_schedule change task.\n");
        _os_current_task->status = READY;
        _os_node_t **head = _os_ready_queue + _os_current_task->priority;
        _os_add_node_tail(head, &(_os_current_task->node));
        _os_set_ready(_os_current_task->priority);
    }

    if (_os_current_task != NULL) {
        PRINT("eos_schedule save task.\n");
		addr_t saved_sp = _os_save_context();

		if (saved_sp == 0) {
			return;
		} else {
			_os_current_task->sp = saved_sp;
		}
	}

    _os_node_t **head = _os_ready_queue + _os_get_highest_priority();
    _os_current_task = (eos_tcb_t *) (*head)->ptr_data;
    _os_remove_node(head, (*head));
 
    if (*head == NULL) {
    	_os_unset_ready(_os_current_task->priority);
    } 

    PRINT("eos_schedule restore task.\n");
    _os_current_task->status = RUNNING;
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
	task->period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
    PRINT("eos_sleep.\n");
    int32u_t timeout = (eos_get_system_timer()->tick) + (eos_get_current_task()->period);
    eos_get_current_task()->status = WAITING;
    eos_set_alarm(eos_get_system_timer(), &(eos_get_current_task()->alarm), timeout, _os_wakeup_sleeping_task, eos_get_current_task());
    eos_schedule();
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
	eos_tcb_t *task = (eos_tcb_t *) arg;
    task->status = READY;
    _os_add_node_tail(_os_ready_queue + task->priority, &(task->node));
    _os_set_ready(task->priority);
}
