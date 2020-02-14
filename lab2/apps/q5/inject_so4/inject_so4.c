#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{

	sem_t s_procs_completed;
	sem_t SO4;
	int mol;
    int i;

	//check for correct no of arg
	if (argc != 4) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	s_procs_completed = dstrtol(argv[1], NULL, 10);
        SO4 = dstrtol(argv[2], NULL, 10);
	mol = dstrtol(argv[3], NULL, 10);

	for(i = 0; i < mol; i++) {
			Printf("SO4 injected into Radeon atmosphere, PID: %d\n", getpid());
			sem_signal(SO4);
	}

	//signal semaphore that we're done
	if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed);
		Printf(argv[0]);
		Printf("exiting..\n");
		Exit();
	}

}
