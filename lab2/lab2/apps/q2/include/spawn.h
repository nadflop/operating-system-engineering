#ifndef _spawn_h_
#define _spawn_h_
#define BUFFER_SIZE 10

#define PRODUCER "producer.dlx.obj"
#define CONSUMER "consumer.dlx.obj"

typedef struct circ_buffer {
	int head;
	int tail;
	char array[BUFFER_SIZE];
}

void buf_enqueue(circ_buffer* buf,char letter);
char buf_dequeue(circ_buffer* buf);
void buf_print(circ_buffer* buf);

#endif
