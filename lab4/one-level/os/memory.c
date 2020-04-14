//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
//freemap is a bit-vector each bit represents whether that page is free or not
static uint32 freemap[16]; //Total memory/pagesize/32 = 16 int32s
static uint32 pagestart;  //Page to allow
static int nfreepages;  //Number of free pages available
static int freemapmax;  //The total number of pages

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {

  int i;
  //-pagestart = first page since last os page
  //Get last address of os
  uint32 last_os_page = (lastosaddress & 0x1FFFFC) / MEM_PAGESIZE; //0x1FFFFC converts to PA
  pagestart = last_os_page + 1; //Page after the os mem that is free

  //-freemapmax = max index for freemap array (ie. 32 bits * 16 pgs)
  freemapmax = MEM_MAX_SIZE / MEM_PAGESIZE;

  //-nfreepages = how many free pages available in the system not including os pages
  nfreepages = MEM_MAX_PAGES - pagestart;

  //-set every entry in freemap to 0 all used
  for(i=0; i<freemapmax/32;i++){
    freemap[i]=0; //TODO: doesn't this clear 32 pages at a
  }

  //Modify freemap to mark the pages are occupied by os
  //-for all free pages:
  //-  MemorySetFreemap()
  for(i=0; i<last_os_page; i++){
    MemorySetFreemap(i,1);
    nfreepages++;
  }
}



//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
//-find pagenum from addr
int page_num = addr / MEM_PAGESIZE;

//-find offset value from addr
int page_offset = addr % MEM_PAGESIZE;

//-check given address is less than the max address
if(addr > MEM_MAX_VIRTUAL_ADDRESS))
  printf("Error in TranslateAddr: input addr exceeds max virtual addr\n");
  ProcessKill();
  return 0;

//-lookup PTE in pcb’s page table



//-if PTE is not valid
//- save addr to the currentSavedFrame
//- page fault handler

//-get and return physical address
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
//-grab faulting addr from the currentSavedFrame

//-grab user stack addr from the currentSavedFrame

//-find pagenum for the faulting addr

//-find pagenum for the user stack

//-segfault (kill the process) if the faulting address is not part of the stack
//-else allocate a new physical page, setup its PTE, and insert to pagetable
//-  return MEM_FAIL;
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {
  //-return 0 if no free pages

  //-find the available bit in freemap

  //-set it to unavailable

  //-decrement number of free pages

  //-return this allocated page number
  return -1;
}

//---------------------------------------------------------------------
// Sets the bit position of the selected page to val(1 or 0)
// to indicate the page is used
//---------------------------------------------------------------------

void MemorySetFreemap(uint32 page, uint32 val){
  uint32 idx = page / 32; //which uint32 it belongs into
  uint32 bit_pos = page % 32 //which bit position in the uint32
  
  //build mask
  uint32 msk = invert(1<<bit_pos);
  freemap[idx] = (freemap[idx] & msk) | (val<<bit_pos);
  return;
}

uint32 MemorySetupPte (uint32 page) {
  return -1;
}

//---------------------------------------------------------------------
// Sets the bit position of the selected page to 0
// to indicate the page is free
//---------------------------------------------------------------------

void MemoryFreePage(uint32 page) {
  MemorySetFreemap(page, 1);
  nfreepages++;
}

