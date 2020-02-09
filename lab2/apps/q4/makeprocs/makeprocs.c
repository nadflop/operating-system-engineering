#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 0;               // Used to store number of processes to create
  int i;                          // Loop index variable
  circ_buffer *buf;               // Used to get address of shared memory page of circular buffer
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  lock_t buff_lock;               // Lock for the buffer
  char str_buff_lock[10];
  cond_t cond_full;				  // Condtion variable used to handle the checking if buffer is full
  char str_cond_full[10];		  //Used as command-line argument
  cond_t cond_empty;		      // Condition variable used to handle the checking if buffer is empty
  char str_cond_empty[10];		  //Used as command-line argument

  if (argc != 2) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numprocs = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d processes\n", numprocs);

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // Map shared memory page into this process's memory space
  if ((buf = (circ_buffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }

  // Initialize circular buffer head and tail
  buf->head = 0;
  buf->tail = 0;

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

  //Create lock for circular buffer
  if ((buff_lock = lock_create()) == SYNC_FAIL){
    Printf("Bad lock_create in");Printf(argv[0]); Printf("\n");
    Exit();
  }
  
  //create conditional variables
  if ((cond_full = cond_create(buff_lock)) == SYNC_FAIL){
    Printf("Bad cond_create in");Printf(argv[0]); Printf("\n");
    Exit();
  }

  if ((cond_empty = cond_create(buff_lock)) == SYNC_FAIL){
    Printf("Bad cond_create in");Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(buff_lock, str_buff_lock);
  ditoa(cond_full, str_cond_full);
  ditoa(cond_empty, str_cond_empty);
  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<numprocs; i++) {
    process_create(PRODUCER, h_mem_str, s_procs_completed_str, str_buff_lock, str_cond_full, str_cond_empty, NULL);
    process_create(CONSUMER, h_mem_str, s_procs_completed_str, str_buff_lock, str_cond_full, str_cond_empty, NULL);
    Printf("Processes %d created\n", i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
