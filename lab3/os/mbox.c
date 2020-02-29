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
  return MBOX_FAIL;
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
  return MBOX_FAIL;
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
  return MBOX_FAIL;
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
  int pid = GetCurrentPid(); //get pid
  
  //check if we have valid handle and length
  if (length < 0 || length > MBOX_MAX_MESSAGE_LENGTH) return MBOX_FAIL;
  if (handle < 0 || handle > MBOX_NUM_MBOXES) return MBOX_FAIL;

  //use lock, aquire it here
  if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
  	printf("Unable to aqcuire lock in MboxSend. Pid: %d \n", pid);
	exitsim();
  }

  //proc sanity check, check if mailbox is already opened or not
  //TODO: how to check?

  //if mbox full, wait for not full
  CondHandleWait(mboxes[handle].notfull);

  //get unused buffer  <-- atomic
  for (i=0; i < MBOX_NUM_BUFFERS; i++) {
  	if (mbox_msgs[i].inuse == 0) {
		mbox_msgs[i].inuse = 1;
		curr = i;
		break;
	}
  }
  
  //set length and copy data
  bcopy(message, mbox_msgs[curr].message, length);
  mbox_msgs[curr].length = length;
  mbox_msgs[curr].inuse = 1;
  
  //create link for message
  if ((l = AQueAllocLink(&mbox_msgs[curr])) == NULL) {
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
  return MBOX_FAIL;
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
  return MBOX_FAIL;
}
