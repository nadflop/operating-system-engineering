#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 0; 
  sem_t s_procs_completed;      //Semaphore used to wait until all spawned processes have completed
  mbox_t S2; 			//mailbox for S2
  mbox_t S;				//mailbox for S
  mbox_t CO; 			//mailbox for CO
  mbox_t O2; 			//mailbox for O2
  //mbox_t C2; 			//mailbox for C2
  mbox_t SO4; 			//mailbox for SO4

  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char S2_str[10];
  char S_str[10];
  char CO_str[10];
  char O2_str[10];
  //char C2_str[10];
  char SO4_str[10];

  //Calculate how many times each process needs to iterate
  int numInject_S2 = 0; 	
  int numInject_CO = 0; 	
  int numReact1;		
  int numReact2;		
  int numReact3;
  int i;
  int j;

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }
  
  // Convert string from ascii command line argument to integer number
  numInject_S2 = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  numInject_CO = dstrtol(argv[2], NULL, 10);
  Printf("Creating %d S2s and %d COs\n", numInject_S2, numInject_CO);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //Create a Mailbox for each Molecule
  //S2 Mailbox
  if ((S2 = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create for S2"); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //S Mailbox
  if ((S = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create for S"); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //CO Mailbox
  if ((CO = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create for CO"); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //O2 Mailbox
  if ((O2 = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create for O2"); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //C2 Mailbox
  /*if ((C2 = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create for C2"); Printf(argv[0]); Printf("\n");
    Exit();
  }*/
  //SO4 Mailbox
  if ((SO4 = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create for SO4"); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //Open all mailbox
  if (mbox_open(S2) != MBOX_SUCCESS) {
  	Printf("Couldn't open mailbox for S2\n");
	Exit();
  }
  if (mbox_open(S) != MBOX_SUCCESS) {
  	Printf("Couldn't open mailbox for S\n");
	Exit();
  }
  if (mbox_open(CO) != MBOX_SUCCESS) {
  	Printf("Couldn't open mailbox for CO\n");
	Exit();
  }
  if (mbox_open(O2) != MBOX_SUCCESS) {
  	Printf("Couldn't open mailbox for O2\n");
	Exit();
  }
 /* if (mbox_open(C2) != MBOX_SUCCESS) {
  	Printf("Couldn't open mailbox for C2\n");
	Exit();
  }*/
  if (mbox_open(SO4) != MBOX_SUCCESS) {
  	Printf("Couldn't open mailbox for SO4\n");
	Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(S2, S2_str);
  ditoa(S, S_str);
  ditoa(CO, CO_str);
  ditoa(O2, O2_str);
  //ditoa(C2, C2_str);
  ditoa(SO4, SO4_str);

  numReact1		= numInject_S2;
  numReact2		= numInject_CO / 4; 
  //2 S2 from reaction1, 2 o2 from reaction2
  numReact3 = min(2*numReact1, numReact2);
  Printf("S2: %d, CO: %d, 1: %d, 2: %d, 3: %d\n", numInject_S2, numInject_CO, numReact1, numReact2, numReact3);
  //need to calculate numprocs?
  //TODO: check if we need to compute numprocs?
  numprocs = numInject_S2 + numInject_CO + numReact1 + numReact2 + numReact3;

  // Now we can create the processes.  Note that you MUST :qend your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.

  while (j < numprocs) {
  	for (i = 0; i < numInject_S2; i ++) {
  		//if (i < numInject_S2) {
  		process_create(INJECT_S2, 0, 0, S2_str, s_procs_completed_str, NULL);
		j++;
  	}
  	for (i = 0; i < numInject_CO; i ++) {
  		//if (i < numInject_CO) {
  		process_create(INJECT_CO, 0, 0, CO_str, s_procs_completed_str, NULL);
		j++;
  	}
  	for (i = 0; i < numReact1; i ++) {
  		//if (i < numReact1) {
		Printf("it got in here1: %d\n", getpid());
  		process_create(REACTION_1, 0, 0, S2_str, S_str, s_procs_completed_str, NULL);
		j++;
  	}
  	for (i = 0; i < numReact2; i ++) {
  		//if (i < numReact2) {
		Printf("it got in here2: %d\n", getpid());
  		process_create(REACTION_2, 0, 0, CO_str, O2_str, s_procs_completed_str, NULL);
		j++;
  	}
  	for (i = 0; i < numReact3; i ++) {
		Printf("it got in here3: %d\n", getpid());
  		process_create(REACTION_3, 0, 0, S_str, O2_str, SO4_str, s_procs_completed_str, NULL);
		j++;
  	}
  }
  

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //Close all mailbox
  if (mbox_close(S2) != MBOX_SUCCESS) {
  	Printf("Couldn't close mailbox for S2\n");
	Exit();
  }
  if (mbox_close(S) != MBOX_SUCCESS) {
  	Printf("Couldn't close mailbox for S\n");
	Exit();
  }
  if (mbox_close(CO) != MBOX_SUCCESS) {
  	Printf("Couldn't close mailbox for CO\n");
	Exit();
  }
  if (mbox_close(O2) != MBOX_SUCCESS) {
  	Printf("Couldn't close mailbox for O2\n");
	Exit();
  }
  /*if (mbox_close(C2) != MBOX_SUCCESS) {
  	Printf("Couldn't close mailbox for C2\n");
	Exit();
  }*/
  if (mbox_close(SO4) != MBOX_SUCCESS) {
  	Printf("Couldn't close mailbox for SO4\n");
	Exit();
  }

  Printf("All other processes completed, exiting main process.\n");
  Printf("%d SO4's created.\n", numReact3);
}
