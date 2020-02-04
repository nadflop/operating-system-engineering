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

	//check for number of arguments
	//convert the command line str to int
	//map shared memory page
	//aquire the lock for producer
	//add char to buff
	//check if buff is full or not
	//release the lock after char is add
	//signal semaphore that we're done

}
