#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int num_H2O = 0;               // Used to store number of H2O to create
  int num_SO4 = 0;               // Used to store number of SO4 to create
  int numprocs = 5; 
  sem_t s_procs_completed;      //Semaphore used to wait until all spawned processes have completed
  sem_t H2O; 			//semaphore used to signal a H2O is created
  sem_t SO4; 			//semaphore used to signal a SO4 is created
  sem_t H2; 			//semaphore used to signal a H2 is created
  sem_t O2; 			//semaphore used to signal a O2 is created
  sem_t SO2; 			//semaphore used to singal a SO2 is created
  sem_t H2SO4; 			//semaphore used to signal a H2SO4 is created


  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char H2O_str[10];
  char SO4_str[10];
  char H2_str[10];
  char O2_str[10];
  char SO2_str[10];
  char H2SO4_str[10];

  //Calculate how many times each process needs to iterate
  int numInject_H2O; 	
  int numInject_SO4; 	
  int numReact1;		
  int numReact2;		
  int numReact3;

  char numInject_H2O_str[10]; 	
  char numInject_SO4_str[10]; 	
  char numReact1_str[10];		
  char numReact2_str[10];		
  char numReact3_str[10];		 

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }
  
  // Convert string from ascii command line argument to integer number
  num_H2O = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  num_SO4 = dstrtol(argv[2], NULL, 10);
  Printf("Creating %d H2Os and %d S04s\n", num_H2O, num_SO4);

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

  //Create a Semaphore for each Molecule
  //H2O Sem
  if ((H2O = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create for H2O"); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //SO4 Sem
  if ((SO4 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create for SO4"); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //H2 Sem
  if ((H2 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create for H2"); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //O2 Sem
  if ((O2 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create for O2"); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //SO2 Sem
  if ((SO2 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create for SO2"); Printf(argv[0]); Printf("\n");
    Exit();
  }

  //H2SO4 Sem
  if ((H2SO4 = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create for H2SO4"); Printf(argv[0]); Printf("\n");
    Exit();
  }


  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(H2O, H2O_str);
  ditoa(SO4, SO4_str);
  ditoa(H2, H2_str);
  ditoa(O2, O2_str);
  ditoa(SO2, SO2_str);
  ditoa(H2SO4, H2SO4_str);


  numInject_H2O 	= num_H2O;
  numInject_SO4 	= num_SO4;
  numReact1		= num_H2O / 2;
  numReact2		= num_SO4;
  numReact3		= min(2*numInject_H2O, min(numInject_H2O + num_SO4, num_SO4)); 

  ditoa(numInject_H2O, numInject_H2O_str);
  ditoa(numInject_SO4, numInject_SO4_str);
  ditoa(numReact1, numReact1_str);
  ditoa(numReact2, numReact2_str);
  ditoa(numReact3, numReact3_str);

  // Now we can create the processes.  Note that you MUST :qend your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  process_create(INJECT_H2O, s_procs_completed_str, H2O_str, numInject_H2O_str, NULL);
  process_create(INJECT_SO4, s_procs_completed_str, SO4_str, numInject_SO4_str, NULL);
  process_create(REACTION_1, s_procs_completed_str, H2O_str, H2_str, O2_str, numReact1_str, NULL);
  process_create(REACTION_2, s_procs_completed_str, SO4_str, SO2_str , O2_str, numReact2_str, NULL);
  process_create(REACTION_3, s_procs_completed_str, H2_str, O2_str, SO2_str, H2SO4_str, numReact3_str, NULL);
    
  

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
