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
		// put semaphore 획득 성공
		// 메시지큐의 마지막 위치(rear)에 메시지를 copy한다
		memcpy(mq->rear, message, mq->msg_size);
		// 메세지큐에서 넣은 메세지 만큼 rear를 이동 시킨다.
		mq->rear = mq->rear + mq->msg_size;

		// 메세지 큐의 마지막 위치를 구한다.
		void *mq_end_position = mq->queue_start + (mq->queue_size - 1) * (mq->msg_size);
		// rear 가 메시지 큐의 마지막 위치를 넘어가면 큐 시작 부분으로 넘겨서 순환큐를 구현한다.
		if (mq->rear > mq_end_position) {
			mq->rear = mq->queue_start;
		}
		
		// get semaphore를 반환한다.
		eos_release_semaphore(&(mq->getsem));
		return 1;	
	} else {
		// put semaphore 획득 실패
		return 0;
	}
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	if (eos_acquire_semaphore(&(mq->getsem), timeout)) {
		// get sempahore 획득 성공
		// 메시지큐의 첫 위치(front)에서 메시지를 copy해 온다.
		memcpy(message, mq->front, mq->msg_size);
		// 메세지큐에서 꺼내간 메세지 만큼 front를 이동 시킨다.
		mq->front = mq->front + mq->msg_size;

		// 메세지 큐의 마지막 위치를 구한다.
		void *mq_end_position = mq->queue_start + (mq->queue_size - 1) * (mq->msg_size);
		// front 가 메시지 큐의 마지막 위치를 넘어가면 큐 시작 부분으로 넘겨서 순환큐를 구현한다.
		if (mq->front > mq_end_position) {
			mq->front = mq->queue_start;
		}
		// put semaphore를 반환한다.
		eos_release_semaphore(&(mq->putsem));
	} else {
		// get sempahore 획득 실패
		return 0;
	}
}
