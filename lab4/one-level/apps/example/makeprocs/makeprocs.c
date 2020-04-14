#include "usertraps.h"
#include "misc.h"

#define Q2_1 "q2_1.dlx.obj"
#define Q2_2 "q2_2.dlx.obj"
#define Q2_3 "q2_3.dlx.obj"
#define Q2_4 "q2_4.dlx.obj"
#define Q2_5 "q2_5.dlx.obj"
#define Q2_6 "q2_6.dlx.obj"

void main (int argc, char *argv[])
{
  int i = 0;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 1) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  // Create Hello World processes (1)
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Creating 1 hello world\n", getpid());
  process_create(Q2_1, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
    // Access memory outside of currently allocated pages (3)
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Access memory outside of currently allocated pages #%d\n", getpid(), i);
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  process_create(Q2_3, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  // Call stack grows larger than 1 page (4)
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Call stack grows larger than 1 page #%d\n", getpid(), i);
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  process_create(Q2_4, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  // Create Hello World processes (5)
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), 100);
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  for(i=0; i<100; i++) {
    Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
    process_create(Q2_5, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
  }

  // Count to a large number in a for loop (6)
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Count to a large number in a for loop for %d times\n", getpid(), 30);
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  for(i=0; i<30; i++) {
    Printf("makeprocs (%d): Count to a large number in a for loop #%d\n", getpid(), i);
    process_create(Q2_6, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
  }
  // Access memory beyond the max virtual address (2)
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Access memory beyond the max virtual address #%d\n", getpid(), i);
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  process_create(Q2_2, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
// #include "usertraps.h"
// #include "misc.h"

// #define HELLO_WORLD "hello_world.dlx.obj"

// void main (int argc, char *argv[])
// {
//   int num_hello_world = 0;             // Used to store number of processes to create
//   int i;                               // Loop index variable
//   sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
//   char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

//   if (argc != 2) {
//     Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
//     Exit();
//   }

//   // Convert string from ascii command line argument to integer number
//   num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
//   Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

//   // Create semaphore to not exit this process until all other processes 
//   // have signalled that they are complete.
//   if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
//     Printf("makeprocs (%d): Bad sem_create\n", getpid());
//     Exit();
//   }

//   // Setup the command-line arguments for the new processes.  We're going to
//   // pass the handles to the semaphore as strings
//   // on the command line, so we must first convert them from ints to strings.
//   ditoa(s_procs_completed, s_procs_completed_str);

//   // Create Hello World processes
//   Printf("-------------------------------------------------------------------------------------\n");
//   Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
//   for(i=0; i<num_hello_world; i++) {
//     Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
//     process_create(HELLO_WORLD, s_procs_completed_str, NULL);
//     if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
//       Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
//       Exit();
//     }
//   }

//   Printf("-------------------------------------------------------------------------------------\n");
//   Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

// }