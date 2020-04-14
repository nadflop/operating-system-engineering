#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int * max_addrs; 

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("q2_3 (%d): Access memory beyond the maximum virtual address!\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("q2_3 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  max_addrs = (MEM_MAX_VIRTUAL_ADDRESS + 1 - MEM_PAGESIZE) - 4;
  //access memory beyond the max virtual address
  Printf("q2_3 (%d): Trying to print val at address %d \n", getpid(), max_addrs);
  Printf("q2_3 (%d): Accessing Memory Location %d \n", getpid(), *max_addrs);
  Printf("q2_3 (%d): Done!\n", getpid());
}
