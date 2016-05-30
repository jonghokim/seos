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
    /* handle when a current task was preempted */
    if (_os_current_task != NULL && _os_current_task->status == RUNNING) {
        _os_current_task->status = READY;

        _os_node_t **head = _os_ready_queue + _os_current_task->priority;
        _os_add_node_tail(head, &(_os_current_task->node));

        _os_set_ready(_os_current_task->priority);
    }

    /* save the current context and store the returned stack pointer */
    if (_os_current_task != NULL) {
        addr_t sav_ctx_sp = _os_save_context();

        /* restored task will be executed from here with the value 0 */
        if (sav_ctx_sp == 0) return;

        _os_current_task->sp = sav_ctx_sp;
    }

    /* Multi-level scheduling */
    /* fetch a task from a highest prioirty ready queue and remove a node of the task */
    _os_node_t **head = _os_ready_queue + _os_get_highest_priority();
    _os_current_task = (eos_tcb_t *)(*head)->ptr_data;
    if (_os_remove_node(head, (*head)) == 0) {
        PRINT("ERROR: _os_remove_node() returned 0\n");
    }
    /* unmask the bitmap for in case of the queue being empty */
    if ((*head) == NULL) _os_unset_ready(_os_current_task->priority);

    /* dispatch the next task via calling _os_restore_context() */
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
    eos_tcb_t *tsk = _os_current_task;
    eos_counter_t *cnter = eos_get_system_timer();
    int32u_t timeout = (cnter->tick) + (tsk->period);

    /* make a status of the task WAITING and set an alarm of the task */
    tsk->status = WAITING;
    eos_set_alarm(cnter, &(tsk->alarm), timeout, _os_wakeup_sleeping_task, tsk);

    /* rescheduling */
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
	eos_tcb_t *tsk = (eos_tcb_t *)arg;
    
    /* set the task READY, put into the ready queue and mask the bitmap */
    tsk->status = READY;
    _os_add_node_tail(_os_ready_queue+tsk->priority, &(tsk->node));
    _os_set_ready(tsk->priority);
}
