/********************************************************
 * Filename: core/comm.c
 *  
 * Author: jtlim, RTOSLab. SNU.
 * 
 * Description: message queue management. 
 ********************************************************/
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
	/* initialization */
	mq->queue_start = queue_start;
	mq->front = queue_start;
	mq->rear = queue_start;
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->queue_type = queue_type;
	
	eos_init_semaphore(&(mq->putsem), queue_size, queue_type);
	eos_init_semaphore(&(mq->getsem), 0, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	if (eos_acquire_semaphore(&(mq->putsem), timeout)) {
		memcpy(mq->rear, message, mq->msg_size);
		mq->rear = mq->rear + mq->msg_size;
		void *last_elem_pos = mq->queue_start + (mq->queue_size - 1)*(mq->msg_size);
		if (mq->rear > last_elem_pos) {
			mq->rear = mq->queue_start;
		}
		// PRINT("sm Im %d, %d\n", eos_get_current_task()->period, eos_get_current_task()->status);
		eos_release_semaphore(&(mq->getsem));
		return 1;	
	} else {
		return 0;
	}
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	if (eos_acquire_semaphore(&(mq->getsem), timeout)) {
		memcpy(message, mq->front, mq->msg_size);
		mq->front = mq->front + mq->msg_size;
		void *last_elem_pos = mq->queue_start + (mq->queue_size - 1)*(mq->msg_size);
		if (mq->front > last_elem_pos) {
			mq->front = mq->queue_start;
		}
		eos_release_semaphore(&(mq->putsem));
	} else {
		return 0;
	}
}
