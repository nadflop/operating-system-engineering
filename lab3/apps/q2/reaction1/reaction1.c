#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	sem_t s_procs_completed;
        sem_t H2O;
	sem_t H2;
	sem_t O2;
	int numreact;
        int i;
 
    //check for correct no of arg
	if (argc != 6) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	s_procs_completed = dstrtol(argv[1], NULL, 10);
	H2O = dstrtol(argv[2], NULL, 10);
	H2 = dstrtol(argv[3], NULL, 10);
	O2 = dstrtol(argv[4], NULL, 10);
	numreact = dstrtol(argv[5], NULL, 10);
	
	for (i = 0; i < numreact; i++) {
		sem_wait(H2O);
		sem_wait(H2O);
		sem_signal(H2);
		sem_signal(H2);
		sem_signal(O2);
		Printf("2 H2O -> 2 H2 + O2 reacted, PID:%d\n", getpid());
	}

	//signal semaphore that we're done
	if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed);
		Printf(argv[0]);
		Printf("exiting..\n");
		Exit();
	}
}
