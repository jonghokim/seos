/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY       1
#define RUNNING     2
#define WAITING     3

/**
 * In 2nd assignment, there would be only three tasks, which are
 *   1. idle task
 *   2. task wrapped of print_number
 *   3. task wrapped of print_alphabet
 *
 * Every should-be-implemented functions in 2nd assignment
 * are implemented in a context of above.
 */
static eos_tcb_t *mytask1 = 0;
static eos_tcb_t *mytask2 = 0;
static eos_tcb_t *mytask3 = 0;

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

/**
 * Temporary implementation for 2nd assignment.
 */
int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
    PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

    task->priority = priority;
    // task->period = 0;         /* not used in 2nd assignment */
    task->status = 0;         /* not used in 2nd assignment */
    task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);

    if (mytask1 == NULL) {
        mytask1 = task;
    } else if (mytask2 == NULL) {
        mytask2 = task;
    } else if (mytask3 == NULL) {
        mytask3 = task;
    } else {
        return -1;  /* error return code */
    }

    return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

/**
 * Temporary implementation for 2nd assignment.
 */
void eos_schedule() {
    /* save the current context and store the returned stack pointer */
    if (_os_current_task != NULL) {
        addr_t sav_ctx_sp = _os_save_context();

        /* restored task will be executed from here with the value 0 */
        if (sav_ctx_sp == 0) return;

        _os_current_task->sp = sav_ctx_sp;
    }

    /* scheduling */
    if (_os_current_task == NULL) {
        _os_current_task = mytask2;
    } else if (_os_current_task == mytask2) {
        _os_current_task = mytask3;
    } else if (_os_current_task == mytask3) {
        _os_current_task = mytask2;
    } else {
        printf("ERROR: eos_schedule() invalid current task\n");
        return -1;      /* error return code */
    }

    /* dispatch the next task via calling _os_restore_context() */
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
