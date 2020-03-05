#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	sem_t s_procs_completed;
    mbox_t CO;
	mbox_t O2;
	mbox_t C2;
	char mes1[] = "O2";
	char mes2[] = "C2";
	char buffer[5];
	int co_rcv1;
	int co_rcv2;
	int co_rcv3;
	int co_rcv4;

    //check for correct no of arg
	if (argc != 5) {
		Printf("Usage");
		Printf(argv[0]);
		Printf("<handle_to_shared_memory_page> <handle_to_page_mapped_semaphore<handle to lock>");
		Exit();
	}

	//convert command line str to int
	s_procs_completed = dstrtol(argv[1], NULL, 10);
	CO = dstrtol(argv[2], NULL, 10);
	O2 = dstrtol(argv[3], NULL, 10);
	C2 = dstrtol(argv[4], NULL, 10);

	//open CO mailbox
	if (mbox_open(CO) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't open CO mailbox\n", getpid());
		Exit();
	}
	//receive 4 CO
	co_rcv1 = mbox_recv(CO, 2, (void *) &buffer);
	if (co_rcv1 != 2) {
		Printf("Reaction 2: %d. Couldn't receive CO\n", getpid());
		Exit();
	}
	co_rcv2 = mbox_recv(CO, 2, (void *) &buffer);
	if (co_rcv2 != 2) {
		Printf("Reaction 2: %d. Couldn't receive CO\n", getpid());
		Exit();
	}
	co_rcv3 = mbox_recv(CO, 2, (void *) &buffer);
	if (co_rcv3 != 2) {
		Printf("Reaction 2: %d. Couldn't receive CO\n", getpid());
		Exit();
	}
	co_rcv4 = mbox_recv(CO, 2, (void *) &buffer);
	if (co_rcv4 != 2) {
		Printf("Reaction 2: %d. Couldn't receive CO\n", getpid());
		Exit();
	}
	//close CO mailbox
	if (mbox_close(CO) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't close CO mailbox\n", getpid());
		Exit();
	}

	//open O2 mailbox
	if (mbox_open(O2) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't open O2 mailbox\n", getpid());
		Exit();
	}
	//send 2 O2
	if (mbox_send(O2, sizeof(mes1), (void*) mes1) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't send O2 mailbox\n", getpid());
		Exit();
	}
	if (mbox_send(O2, sizeof(mes1), (void*) mes1) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't send O2 mailbox\n", getpid());
		Exit();
	}
	//close O2 mailbox
	if (mbox_close(O2) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't close O2 mailbox\n", getpid());
		Exit();
	}

	//open C2 mailbox
	if (mbox_open(C2) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't open C2 mailbox\n", getpid());
		Exit();
	}
	//send 2 C2
	if (mbox_send(C2, sizeof(mes2), (void*) mes2) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't send C2 mailbox\n", getpid());
		Exit();
	}
	if (mbox_send(C2, sizeof(mes2), (void*) mes2) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't send C2 mailbox\n", getpid());
		Exit();
	}
	//close C2 mailbox
	if (mbox_close(C2) != MBOX_SUCCESS) {
		Printf("Reaction 2: %d. Couldn't close C2 mailbox\n", getpid());
		Exit();
	}

	Printf("CO -> 2 O2 + 2 C2 reacted, PID:%d\n", getpid());	

	//signal semaphore that we're done
	if (sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed);
		Printf(argv[0]);
		Printf("exiting..\n");
		Exit();
	}
}
