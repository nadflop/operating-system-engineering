#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

mbox 		mboxes[MBOX_NUM_MBOXES];
mbox_message 	mbox_msgs[MBOX_NUM_BUFFERS];

//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
	int i;
	int j;
	
	//Initialize all mbox inuse to 0
	for (i = 0; i<MBOX_NUM_MBOXES; i++){
		mboxes[i].inuse = 0;

		for(j=0; j<PROCESS_MAX_PROCS; j++){
			mboxes[i].pid[j] = 0;
		}
	}
	
	//Initialize all msg inuse values to 0
	for(i=0; i<MBOX_NUM_BUFFERS; i++){
		mbox_msgs[i].inuse = 0;
	}
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
	//Grab available mbox atomically
	int avaiable_slot=0;
	while(mboxes[available_slot].inuse == 1){
		available_slot++;
		if(available_slot > MBOX_NUM_BOXES) return MBOX_FAIL;
	}
	
	//attach the lock
	if(mboxes[available_slot].lock = lock_create() == SYNC_FAIL){
    		Printf("Bad lock_create in"); Printf("\n");
    		Exit();
	}
	
	//create conditional variables
	if((mboxes[available_slot].notempty = cond_create(mboxes[available_slot].lock)) == SYNC_FAIL){
    		Printf("Bad cond_create in"); Printf("\n");
    		Exit();
	}

	if((mboxes[available_slot].notfull = cond_create(mboxes[available_slot].lock)) == SYNC_FAIL){
    		Printf("Bad cond_create in"); Printf("\n");
    		Exit();
	}

	//initialize the msg queue
	if((AQueueInit(mboxes[available_slot].msg_queue) == QUEUE_FAIL){
    		Printf("Bad queue init ");Printf("\n");
    		Exit();
	}
	//Check that the mbox hasn't been opened yet
	int i;
	for(i=0; i<PROCESS_MAX_PROCS; i++){
		if(mboxes[available_slot].procs[i] == 1){
			Printf("Error: Process has already opened mailbox\n");			
			return MBOX_FAIL;
		}
	}
	
	return MBOX_SUCCESS;
}

//-------------------------------------------------------
// 
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
	int pid = 0;
	int mbox_lock;
	pid = getpid();

	if(handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;


	if (lock_acquire(mbox[handle].lock) != SYNC_SUCCESS) {
		Exit();
	}
	mbox[handle].procs[pid] = 1;
	
	if (lock_release(mbox[handle].lock) != SYNC_SUCCESS) {
		Exit();
	}
	
	return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
	int pid = 0;
	int mbox_lock;
	pid = getpid();
	Link* link;

	//Check handle value is valid
	if(handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;

	//Acquire lock
	if (lock_acquire(mbox[handle].lock) != SYNC_SUCCESS) {
		Exit();
	}
	//Set the current process index to 0
	mbox[handle].procs[pid] = 0;
	
	//Clear the Queue
	while(!AQueueEmpty(&mboxes[handle].msg_queue){		//Check if the link is empty
		link = AQueueFirst(&mboxes[handle].msg_queue); 	//Get the first link
		AQueueRemove(&link);				//Remove the link
	}
	
	mboxes[handle].inuse = 0;

	//Release the lock
	if (lock_release(mbox[handle].lock) != SYNC_SUCCESS) {
		Exit();
	}
	
	return MBOX_SUCCESS;
  	
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
  
  int i;
  int curr = 0;
  Link * l;
  int currpid = GetCurrentPid(); //get pid
  
  //check if we have valid handle and length
  if (length < 0 || length > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;
  if (handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;

  //check if mailbox is already opened or not
  if (mboxes[handle].procs[currpid] != 1) {
  	return MBOX_FAIL;
  }

  if (mboxes[handle].inuse != 1) {
  	return MBOX_FAIL;
  }

  //if mbox full, wait for not full
  //TODO: check if this makes sense or not later
  //if it doesn't work, just add a new variable to keep track no of msg in mailbox
  if ( AQueueLength(&mboxes[handle].msg_queue)  >= MBOX_MAX_BUFFERS_PER_MBOX) {
  	CondHandleWait(mboxes[handle].notfull);
  }

  //use lock, aquire it here
  if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to aqcuire lock in MboxSend. Pid: %d \n", currpid);
	exitsim();
  }

  //get unused buffer  <-- atomic
  for (i=0; i < MBOX_NUM_BUFFERS; i++) {
  	if (mbox_msgs[i].inuse == 0) {
		mbox_msgs[i].inuse = 1;
		curr = i;
		break;
	}
  }
  
  //set length and copy from "data"
  bcopy(message, mbox_msgs[curr].buffer, length);
  mbox_msgs[curr].length = length;
  mbox_msgs[curr].inuse = 1;
  
  //create link for message
  if ((l = AQueueAllocLink(&mbox_msgs[curr])) == NULL) {
  	printf("ERROR: could not allocate link for message in MboxSend\n");
	exitsim();
  }

  //add msg to queue, use FIFO
  AQueueInsertLast(&mboxes[handle].msg_queue, l);

  //release lock
  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to release lock acquired. Pid: %d \n", GetCurrentPid());
	exitsim();
  }

  //signal not empty
  CondHandleSignal(mboxes[handle].notempty);
  
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
  Link * l;
  mbox_message * temp;
  int currpid = GetCurrentPid(); //get pid
  
  //check if we have valid handle and length
  if (maxlength > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;
  if (handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;

  //check if mailbox is already opened or not
  if (mboxes[handle].procs[currpid] != 1) {
  	return MBOX_FAIL;
  }

  if (mboxes[handle].inuse != 1) {
  	return MBOX_FAIL;
  }

  //if mbox empty, wait for not empty
  //TODO: check if this makes sense or not later
  //if it doesn't work, just add a new variable to keep track no of msg in mailbox
  if ( AQueueLength(&mboxes[handle].msg_queue)  == 0) {
  	CondHandleWait(mboxes[handle].notempty);
  }
  
  //use lock, aquire it here
  if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to aqcuire lock in MboxSend. Pid: %d \n", currpid);
	exitsim();
  }
  
  //mailbox is FIFO
  //get first message in the list
  l = AQueueFirst(&(mboxes[handle].msg_queue));
  temp = (mbox_message *) l->object;

  //check if message buffer length is longer/larger than maxlength
  if (temp->length > maxlength) {
  	printf("ERROR: size of message in buffer (%d) > maxlength (%d)", temp->length, maxlength);
  	return MBOX_FAIL;
  }

  //copy message to "data"
  bcopy(temp->buffer, message, temp->length);
  temp->inuse = 0;

  //remove msg from queue, use FIFO
  AQueueRemove(&l);

  //release lock
  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to release lock acquired. Pid: %d \n", GetCurrentPid());
	exitsim();
  }

  //signal not full
  CondHandleSignal(mboxes[handle].notfull);
  
  return temp->length;

}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
  int i;
  int j;
  int numprocs;
  Link * l;
  //use lock
  //loop through all mboxes. if pid opened it-> remove from list
  //if no other procs using the box -> mark not in use
  
  for (i = 0; i < MBOX_NUM_MBOXES; i++) {
    //check if the mailbox is opened by the pid
  	if (mboxes[i].procs[pid] == 1) {
		//use lock, aquire it here
  	    if (LockHandleAcquire(mboxes[i].lock) != SYNC_SUCCESS) {
  			printf("Unable to aqcuire lock in MboxSend. Pid: %d \n", pid);
			exitsim();
  		}
		
		//check if there are other process
		mboxes[i].procs[pid] = 0;
	    for (j = 0; j < PROCESS_MAX_PROCS; j++) {
			if (mboxes[i].procs[j] == 1) {
				numprocs++;
			}
		}
		
		//if there's only one process, mark mboxes as unused
		if (numprocs == 1) {
			//remove all queued up messages
			while (!AQueueEmpty(&mboxes[i].msg_queue)) {
				l = AQueueFirst(&(mboxes[i].msg_queue));
				AQueueRemove(&l);
			}
			//mark mboxes as unused
			mboxes[i].inuse = 0;
		}

		//release lock
  		if (LockHandleRelease(mboxes[i].lock) != SYNC_SUCCESS) {
  			printf("Unable to release lock acquired. Pid: %d \n", GetCurrentPid());
			exitsim();
  		}
	}
  }

  return MBOX_SUCCESS;
}
