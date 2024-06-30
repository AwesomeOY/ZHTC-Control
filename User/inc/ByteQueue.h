#ifndef BYTE_QUEUE_H_
#define BYTE_QUEUE_H_

typedef struct {
	unsigned char* buff;
	int len;
	int front;
	int rear;
}Queue;

void queue_init(Queue* pq, unsigned char* buf, int len);

int queue_len(Queue* pq);

unsigned char queue_is_full(Queue* pq);

unsigned char queue_en(Queue* pq, unsigned char data);

unsigned char queue_de(Queue* pq, unsigned char* data);

#endif
