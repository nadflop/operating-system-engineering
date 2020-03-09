#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{

	mbox_t s_procs_completed;
	mbox_t S2;
	char mes[] = "S2";
    int i;

	//check for correct no of arg
	if (argc != 3) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	s_procs_completed = dstrtol(argv[2], NULL, 10);
    S2 = dstrtol(argv[1], NULL, 10);

    //open mailbox
	if (mbox_open(S2) != MBOX_SUCCESS) {
		Printf("Injection S2 %d Couldn't open mbox\n", getpid());
		Exit();
	}
	//send mail
	if (mbox_send(S2, 2, (void *) &mes) != MBOX_SUCCESS) {
		Printf("Injection S2: %d Couldn't send mbox\n", getpid());
		Exit();
	}

	Printf("S2 injected into Radeon atmosphere, PID: %d\n", getpid());
	//close mailbox
	if (mbox_close(S2) != MBOX_SUCCESS) {
		Printf("Injection S2: %d Couldn't close mbox\n", getpid());
		Exit();
	}

	//signal semaphore that we're done
	if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed);
		Printf(argv[0]);
		Printf("exiting..\n");
		Exit();
	}

}
