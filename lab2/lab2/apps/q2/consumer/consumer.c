#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	circ_buffer * buff;
	uint32 h_mem;
	sem_t s_procs_completed;
	lock_t buff_lock;

	//check for correct no of arg
	//convert command line str to int
	//map shared memory page into this process memory page
	//aquire the lock for the process
	//check if buffer is empty or not before removing a char
	//remove a char if its not empty
	//releas the lock
	//signal semaphore tell that we're done
}
