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
			mboxes[i].procs[j] = 0;
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
	int available_slot=0;
	int i =0;
	while(mboxes[available_slot].inuse == 1){
		available_slot++;
		if(available_slot > MBOX_NUM_MBOXES) return MBOX_FAIL;
	}
	
	//attach the lock
	if((mboxes[available_slot].lock = LockCreate()) == SYNC_FAIL){
    		printf("Bad lock_create in\n"); 
    		return MBOX_FAIL;
	}
	
	//create conditional variables
	if((mboxes[available_slot].notempty = CondCreate(mboxes[available_slot].lock)) == SYNC_FAIL){
    		printf("Bad cond_create in"); printf("\n");
    		return MBOX_FAIL;
	}

	if((mboxes[available_slot].notfull = CondCreate(mboxes[available_slot].lock)) == SYNC_FAIL){
    		printf("Bad cond_create in"); printf("\n");
    		return MBOX_FAIL;
	}

	//initialize the msg queue
	if(AQueueInit(&mboxes[available_slot].msg_queue) == QUEUE_FAIL){
    		printf("Bad queue init ");printf("\n");
    		return MBOX_FAIL;
	}
	//Check that the mbox hasn't been opened yet
	
	for(i=0; i<PROCESS_MAX_PROCS; i++){
		if(mboxes[available_slot].procs[i] == 1){
			printf("Error: Process has already opened mailbox\n");			
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
	pid = GetCurrentPid();

	if(handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;


	if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
		return MBOX_FAIL;
	}
	mboxes[handle].procs[pid] = 1;
	
	if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS) {
		return MBOX_FAIL;
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
	int pid = GetCurrentPid();
	Link * link;

	//Check handle value is valid
	if(handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;

	//Acquire lock
	if (LockHandleAcquire(mboxes[handle].lock) == SYNC_FAIL) {
		return MBOX_FAIL;
	}
	//Set the current process index to 0
	mboxes[handle].procs[pid] = 0;
	
	//Clear the Queue
	while(!AQueueEmpty(&mboxes[handle].msg_queue)) {		//Check if the link is empty
		link = AQueueFirst(&mboxes[handle].msg_queue);//) == NULL) { 	//Get the first link
		//	printf("Unable to get first link in MboxClose. Pid : %d\n", pid);
		//	return MBOX_FAIL;
		//}
		if (AQueueRemove(&link) != QUEUE_SUCCESS) {				//Remove the link
			printf("fail to remove message in queue in MboxClose : %d, pid : %d\n", handle, pid);
			return MBOX_FAIL;
		}
	}
	mboxes[handle].inuse = 0;

	//Release the lock
	if (LockHandleRelease(mboxes[handle].lock) == SYNC_FAIL) {
		return MBOX_FAIL;
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
  //JUST ADDED THIS
  //uint32 intrval;
  
  //check if we have valid handle and length
  if (length < 0 || length > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;
  if (handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;

  //check if mailbox is already opened or not
  if (mboxes[handle].procs[currpid] != 1) {
  	return MBOX_FAIL;
  }

  //use lock, aquire it here
  if (LockHandleAcquire(mboxes[handle].lock) == SYNC_FAIL) {
  	printf("Unable to aqcuire lock in MboxSend. Pid: %d \n", currpid);
	//exitsim();
	return MBOX_FAIL;
  }
  
  //if mbox full, wait for not full
  while (mboxes[handle].msg_queue.nitems  >= MBOX_MAX_BUFFERS_PER_MBOX) {
  	printf("PID(send): %d\n", GetCurrentPid());
	if (CondHandleWait(mboxes[handle].notfull) != SYNC_SUCCESS){
		printf("Bad condWait in MboxSend() for mbox: %d, pid: %d\n", handle, currpid);
		//exitsim();
		return MBOX_FAIL;	
	}
  }

  //intrval = DisableIntrs();
  //get unused buffer  <-- atomic
  for (i=0; i < MBOX_NUM_BUFFERS; i++) {
  	if (mbox_msgs[i].inuse == 0) {
		mbox_msgs[i].inuse = 1;
		curr = i;
		break;
	}
  }
  //RestoreIntrs(intrval);
 
  mbox_msgs[curr].length = length;
  //set length and copy from "data"
  bcopy(message, (void*) mbox_msgs[curr].buffer, length);
  mbox_msgs[curr].inuse = 1;
  
  //create link for message
  l = AQueueAllocLink(&mbox_msgs[curr].buffer);//) == NULL) {
  //	printf("ERROR: could not allocate link for message in MboxSend\n");
//	return MBOX_FAIL;
  //}

  //add msg to queue, use FIFO
  if (AQueueInsertLast(&mboxes[handle].msg_queue, l) != QUEUE_SUCCESS) {
  	printf("Unable to insert message to the queue in MboxSend\n");
	//exitsim();
	return MBOX_FAIL;
  }

  //signal not empty
  if (CondHandleSignal(mboxes[handle].notempty) == SYNC_FAIL){
		printf("Bad condSignal in MboxSend() for mbox: %d, pid: %d", handle, currpid);
		//exitsim();
		return MBOX_FAIL;
  }

  //release lock
  if (LockHandleRelease(mboxes[handle].lock) == SYNC_FAIL) {
  	printf("Unable to release lock acquired. Pid: %d \n", GetCurrentPid());
	//exitsim();
	return MBOX_FAIL;
  }

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
  int size = 0;
  int currpid = GetCurrentPid(); //get pid
  
  //check if we have valid handle and length
  if (maxlength > MBOX_MAX_MESSAGE_LENGTH) {
  	printf("maxlength problem");
	return MBOX_FAIL;
  }
  if (handle < 0 || handle > MBOX_NUM_MBOXES) {
  	printf("handle problem");
	return MBOX_FAIL;
  }

  //check if mailbox is already opened or not
  if (mboxes[handle].procs[currpid] != 1) {
  	printf("mailbox unopened?");
	//exitsim();
	return MBOX_FAIL;
  }

  //use lock, aquire it here
  if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to aqcuire lock in MboxSend. Pid: %d \n", currpid);
	//exitsim();
	return MBOX_FAIL;
  }

  //if mbox empty, wait for not empty
  while ( AQueueEmpty(&mboxes[handle].msg_queue)) {
	if (CondHandleWait(mboxes[handle].notempty) != SYNC_SUCCESS){
		printf("Bad condWait in MboxRecv() for mbox: %d, pid: %d\n", handle, currpid);
		//exitsim();
		return MBOX_FAIL;
	}
  }
  
  //mailbox is FIFO
  //get first message in the list
  l = AQueueFirst(&(mboxes[handle].msg_queue));//) == NULL) {
  //  printf("Unable to get first message in queue in MboxRecv\n");
  //  return MBOX_FAIL;
  //}
  temp = (mbox_message *) l->object;

  //check if message buffer length is longer/larger than maxlength
  if (temp->length > maxlength) {
  	printf("ERROR: size of message in buffer (%d) > maxlength (%d)", temp->length, maxlength);
  	return MBOX_FAIL;
  }

  //copy message to "data"
  bcopy((void*) temp->buffer, message, temp->length);
  size = ((mbox_message *)(AQueueObject(l)))->length;
  printf("MESSAGE SIZE: %d, temp length: %d\n", size, temp->length);
  temp->inuse = 0;

  //remove msg from queue, use FIFO
  if (AQueueRemove(&l) != QUEUE_SUCCESS) {
    printf("Unable to remove message from queue in MboxRecv\n");
	//exitsim();
	return MBOX_FAIL;
  }

  //signal not full
  if (CondHandleSignal(mboxes[handle].notfull) != SYNC_SUCCESS){
		printf("Bad condSignal in MboxRecv() for mbox: %d, pid: %d", handle, currpid);
		//exitsim();
		return MBOX_FAIL;
	}
  //release lock
  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to release lock acquired. Pid: %d \n", GetCurrentPid());
	//exitsim();
	return MBOX_FAIL;
  }
  
  return (size);
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
			return MBOX_FAIL;
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
				l = AQueueFirst(&(mboxes[i].msg_queue));//) == NULL) {
				//	printf("Unable to get first link in MboxCloseAllByPid\n");
				//	return MBOX_FAIL;
				//}
				if (AQueueRemove(&l) != QUEUE_SUCCESS) {
    				printf("Unable to remove message from queue in MboxCloseAllByPid\n");
					return MBOX_FAIL;
				}
			}
			//mark mboxes as unused
			mboxes[i].inuse = 0;
		}

		//release lock
  		if (LockHandleRelease(mboxes[i].lock) != SYNC_SUCCESS) {
  			printf("Unable to release lock acquired. Pid: %d \n", GetCurrentPid());
			return MBOX_FAIL;
  		}
	}
  }

  return MBOX_SUCCESS;
}
