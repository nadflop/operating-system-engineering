#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	sem_t s_procs_completed;
    mbox_t S2;
	mbox_t S;
	char mes[] = "S";
	char buffer[5];
	int s2_rcv;

    //check for correct no of arg
	if (argc != 4) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	s_procs_completed = dstrtol(argv[1], NULL, 10);
	S2 = dstrtol(argv[2], NULL, 10);
	S = dstrtol(argv[3], NULL, 10);
	
	//open S2 mailbox
	if (mbox_open(S2) != MBOX_SUCCESS) {
		Printf("Reaction 1: %d. Couldn't open S2 mailbox\n", getpid());
		Exit();
	}
	//receive 1 S2
	s2_rcv = mbox_recv(S2, 2, (void *) &buffer);
	if (s2_rcv != 2) {
		Printf("Reaction 1: %d. Couldn't receive S2\n", getpid());
		Exit();
	}

	//close S2 mailbox
	if (mbox_close(S2) != MBOX_SUCCESS) {
		Printf("Reaction 1: %d. Couldn't close S2 mailbox\n", getpid());
		Exit();
	}

	//open S mailbox
	if (mbox_open(S) != MBOX_SUCCESS) {
		Printf("Reaction 1: %d. Couldn't open S mailbox\n", getpid());
		Exit();
	}

	//send 2 S
	if (mbox_send(S, sizeof(mes), (void*) mes) != MBOX_SUCCESS) {
		Printf("Reaction 1: %d. Couldn't send S mailbox\n", getpid());
		Exit();
	}
	if (mbox_send(S, sizeof(mes), (void*) mes) != MBOX_SUCCESS) {
		Printf("Reaction 1: %d. Couldn't send S mailbox\n", getpid());
		Exit();
	}

	//close S mailbox
	if (mbox_close(S) != MBOX_SUCCESS) {
		Printf("Reaction 1: %d. Couldn't close S mailbox\n", getpid());
		Exit();
	}

	Printf("S2 -> 2 S reacted, PID:%d\n", getpid());	

	//signal semaphore that we're done
	if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed);
		Printf(argv[0]);
		Printf("exiting..\n");
		Exit();
	}
}
