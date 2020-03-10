#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	sem_t s_procs_completed;
    mbox_t S;
	mbox_t O2;
	mbox_t SO4;
	char mes[] = "SO4";
	char buffer1[5];
	char buffer2[5];

    //check for correct no of arg
	if (argc != 5) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	s_procs_completed = dstrtol(argv[4], NULL, 10);
	S = dstrtol(argv[1], NULL, 10);
	O2 = dstrtol(argv[2], NULL, 10);
	SO4 = dstrtol(argv[3], NULL, 10);

	//open S mailbox
	if (mbox_open(S) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't open S mailbox\n", getpid());
		Exit();
	}	
	//open O2 mailbox
	if (mbox_open(O2) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't open O2 mailbox\n", getpid());
		Exit();
	}
	//open SO4 mailbox
	if (mbox_open(SO4) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't open SO4 mailbox\n", getpid());
		Exit();
	}
	//receive 1 S
	if (mbox_recv(S, 1, (void *) &buffer1) == MBOX_FAIL) {
		Printf("Reaction 3: %d. Couldn't receive S\n", getpid());
		Exit();
	}
	//Printf("Reaction 3 %d: Received %c\n", getpid(), buffer1);

	//receive 2 O2
	if (mbox_recv(O2, 2, (void *) &buffer2) == MBOX_FAIL) {
		Printf("Reaction 3: %d. Couldn't receive O2\n", getpid());
		Exit();
	}
	//Printf("Reaction 3 %d: Received %c\n", getpid(), buffer2);
	if (mbox_recv(O2, 2, (void *) &buffer2) == MBOX_FAIL) {
		Printf("Reaction 3: %d. Couldn't receive O2\n", getpid());
		Exit();
	}
	//Printf("Reaction 3 %d: Received %c\n", getpid(), buffer2);

	//send 1 SO4
	if (mbox_send(SO4, 3, (void*) &mes) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't send SO4 mailbox\n", getpid());
		Exit();
	}
	//close SO4 mailbox
	if (mbox_close(SO4) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't close SO4 mailbox\n", getpid());
		Exit();
	}
	//close S mailbox
	if (mbox_close(S) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't close S mailbox\n", getpid());
		Exit();
	}
	
	Printf("S + 2 O2 -> SO4 reacted, PID:%d\n", getpid());	

	//close O2 mailbox
	if (mbox_close(O2) != MBOX_SUCCESS) {
		Printf("Reaction 3: %d. Couldn't close O2 mailbox\n", getpid());
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
