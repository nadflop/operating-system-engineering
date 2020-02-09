#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	circ_buffer * buf;
	uint32 h_mem;
	sem_t s_procs_completed;
	lock_t buff_lock;
	cond_t cond_full;
	cond_t cond_empty;
	int i = 0;
	char str[] = "Hello World";

	//check for correct no of arg
	if (argc != 6) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	h_mem = dstrtol(argv[1], NULL, 10);
	s_procs_completed = dstrtol(argv[2], NULL, 10);
	buff_lock = dstrtol(argv[3], NULL, 10);
    cond_full = dstrtol(argv[4], NULL, 10);
	cond_empty = dstrtol(argv[5], NULL, 10);

	//map shared memory page into this process memory page
	if ((buf = (circ_buffer *) shmat(h_mem)) == NULL) {
		Printf("Could not map the virtual address");
		Printf(argv[0]);
		Printf("exiting\n");
		Exit();
	}
	
	while (i < dstrlen(str)) {
		//aquire the lock for the process
		if (lock_acquire(buff_lock) != SYNC_SUCCESS) {
			Exit();
		}
		//check if buffer is empty or not before removing a char
		if (buf->head == buf->tail) {
			//buffer is empty
		}
		else {
			//buffer is not full
			Printf("Consumer %d removed: %c\n", getpid(), buf->array[buf->tail]);
			i++;
			buf->tail = (buf->tail + 1) % BUFFERSIZE;
		}
		//release the lock
		if (lock_release(buff_lock) != SYNC_SUCCESS) {
			Exit();
		}
	}

	//signal semaphore that we're done
	Printf("consumer: PID %d is complete.\n", getpid());
	if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed);
		Printf(argv[0]);
		Printf("exiting..\n");
		Exit();
	}
}
