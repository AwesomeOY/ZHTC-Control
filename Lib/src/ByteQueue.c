#include "ByteQueue.h"

void queue_init(Queue* pq, unsigned char* buf, int len)
{
	pq->buff = buf;
	pq->len = len;
	pq->front = 0;
	pq->rear = 0;
}

/* 返回队列中有效数据个数 */
inline int queue_len(Queue* pq)
{
	return (pq->rear - pq->front + pq->len) % pq->len;
}

/* 队列满 判断，满-返回1 */
inline unsigned char queue_is_full(Queue* pq)
{
	if (queue_len(pq) == (pq->len - 1)) {
		return 1;
	}
	return 0;
}

/* 插入数据到队列队尾 */
unsigned char queue_en(Queue* pq, unsigned char data)
{
	if ((pq->rear+1) % pq->len == pq->front) {
		return 0;
	}
	pq->buff[pq->rear++] = data;
	pq->rear %= pq->len;
	return 1;
}

/* 从队列对头读取数据 */
unsigned char queue_de(Queue* pq, unsigned char* data)
{
	if (pq->rear == pq->front) {
		return 0;
	}
	*data = pq->buff[pq->front++];
	pq->front %= pq->len;
	return 1;
}
