#include "spawn.h"

void buf_enqueue(circ_buffer* buf,char letter){
	if ((buf->head +1) % BUFFER_SIZE == buf->tail){ //Buffer full
		return false
	}
	else{
		//increment head
		//check for end of array
		if(buf->head == BUFFER_SIZE - 1){ //head at end of array
			buf->head = 0;
		}
		else{
			buf->head++;
		}

		//add char
		buf->array[buf->head] = letter;
		return true
	}
}


char buf_dequeue(circ_buffer* buf){
	if (buf->head == buf->tail){ //Empty buffer
		return NULL
	}
	else{

		//get letter from array
		char letter = buf->array[buf->tail];
	
		//increment tail
		//check for end of array
		if(buf->tail == BUFFER_SIZE - 1){ //head at end of array
			buf->tail = 0;
		}
		else{
			buf->tail++;
		}

		return letter;	
	}
}


void buf_print(circ_buffer* buf){
	printf("Circular Buffer: [");
	for(int i=0; i<BUFFER_SIZE; i++){
		printf("%c, ",buf->array[i]); 
	}
	printf("]\nHead: %d Tail: %d\n", buf->head, buf->tail);
}
