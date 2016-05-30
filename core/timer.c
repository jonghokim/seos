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
	_os_node_t **alarm_queue = &(counter->alarm_queue);
	_os_node_t *alarm_queue_node = &(alarm->alarm_queue_node);

	_os_remove_node(alarm_queue, alarm_queue_node);
	if (timeout <= 0 || entry == NULL) {
		return;	
	}

	alarm->timeout = timeout;
	alarm->handler = entry;
	alarm->arg = arg;

	alarm_queue_node->ptr_data = alarm;
	alarm_queue_node->priority = timeout;
	_os_add_node_priority(alarm_queue, alarm_queue_node);
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
	counter->tick += 1;
	PRINT("tick %d\n", counter->tick);
	_os_node_t **alarm_queue = &(counter->alarm_queue);
	
	while (*alarm_queue != NULL) {
		eos_alarm_t *alarm = (eos_alarm_t *) (*alarm_queue)->ptr_data;

		if (alarm->timeout > counter->tick) {
			break;
		}

		alarm->handler(alarm->arg);
		eos_set_alarm(&system_timer, alrm, 0, NULL, NULL);
		// _os_remove_node(alarm_queue, alarm);
	}

	eos_schedule();
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	PRINT("initializing timer module.\n");
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
