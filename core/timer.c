/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: 
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

void eos_set_alarm(eos_counter_t* counter, eos_alarm_t* alarm, int32u_t timeout, void (*entry)(void *arg), void *arg) {
	_os_node_t **head = &(counter->alarm_queue);
	_os_node_t *node = &(alarm->alarm_queue_node);

	/* remove the alarm from an alarm queue of the count */
	_os_remove_node(head, node);

	bool_t rls_case = (timeout == 0) || (entry == NULL);   // release case
	if (rls_case) return;

	/* initialize the alarm */
	alarm->timeout = timeout;
	alarm->handler = entry;
	alarm->arg = arg;

    /* initialize a queue node of the alarm and push into the alarm queue in asen order */
	node->ptr_data = alarm;
	node->priority = timeout;
	_os_add_node_priority(head, node);
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
	counter->tick += 1;
	PRINT("tick %d\n", counter->tick);

	_os_node_t **head = &(counter->alarm_queue);
	while ((*head) != NULL) {
		eos_alarm_t *alrm = (eos_alarm_t *)(*head)->ptr_data;

		if (alrm->timeout > counter->tick) break;

		/* call a handler of the timeout-ed alarm, then release it from the queue */
		alrm->handler(alrm->arg);
		eos_set_alarm(&system_timer, alrm, 0, NULL, NULL);
	}

	eos_schedule();
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
