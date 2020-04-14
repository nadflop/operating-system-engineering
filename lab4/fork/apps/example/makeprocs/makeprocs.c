#include "usertraps.h"
#include "misc.h"

#define TEST_FORK "test_fork.dlx.obj"

void main (int argc, char *argv[])
{
  int i = 0;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 1) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }
  
  if ((s_procs_completed = sem_create(1)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  ditoa(s_procs_completed, s_procs_completed_str);
  process_create(TEST_FORK, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());
}