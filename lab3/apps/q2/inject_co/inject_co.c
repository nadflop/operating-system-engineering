#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{

	mbox_t s_procs_completed;
	mbox_t CO;
	char mes[] = "CO";
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
    CO = dstrtol(argv[1], NULL, 10);

    //open mailbox
	if (mbox_open(CO) != MBOX_SUCCESS) {
		Printf("Injection C0: %d Couldn't open mbox\n", getpid());
		Exit();
	}
	//send mail
	if (mbox_send(CO, sizeof(char), (void *) &mes) != MBOX_SUCCESS) {
		Printf("Injection C0: %d Couldn't send mbox\n", getpid());
		Exit();
	}

	Printf("CO injected into Radeon atmosphere, PID: %d\n", getpid());
	//close mailbox
	if (mbox_close(CO) != MBOX_SUCCESS) {
		Printf("Injection CO: %d Couldn't close mbox\n", getpid());
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
