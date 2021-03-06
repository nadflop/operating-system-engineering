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
#include "memory_constants.h"

// num_pages = size_of_memory / size_of_one_page
//freemap is a bit-vector each bit represents whether that page is free or not
static uint32 freemap[MEM_MAX_PAGES/32]; //Total memory/pagesize/32 = 16 int32s
static uint32 pagestart;  //Page to allow
static int nfreepages;  //Number of free pages available
static int freemapmax;  //The total number of pages
static int page_refcounters[ MEM_MAX_PAGES];
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
  uint32 last_os_page = (lastosaddress + MEM_PAGESIZE -4) / MEM_PAGESIZE; //0x1FFFFC converts to PA
  pagestart = last_os_page; //Page after the os mem that is free

  //-freemapmax = max index for freemap array (ie. 32 bits * 16 pgs)
  freemapmax = MEM_MAX_PHYS_MEM / MEM_PAGESIZE;
  //-nfreepages = how many free pages available in the system not including os pages
  nfreepages = 0;

  //-set every entry in freemap to 0 all used
  for(i=0; i<MEM_MAX_PAGES/32;i++){
    freemap[i]=0; //TODO: doesn't this clear 32 pages at a
  }
  
  //Set ref counter to be one for all os pages
  for(i=0; i<pagestart; i++){
    page_refcounters[i] = 1;
  }

  printf("MemoryModInit: os occupies up to pg %d\n", last_os_page);
  //Modify freemap to mark the pages are occupied by os
  //-for all free pages:
  //-  MemorySetFreemap()
  for(i=pagestart; i< MEM_MAX_PHYS_MEM/MEM_PAGESIZE; i++){
    nfreepages++;
    MemorySetFreemap(i,1);
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
  if(addr > MEM_MAX_VIRTUAL_ADDRESS){
    printf("Error in TranslateAddr: input addr exceeds max virtual addr\n");
    ProcessKill();
    return(0);
  }
  //-lookup PTE in pcb’s page table
  //-if PTE is not valid
  //- save addr to the currentSavedFrame
  //- page fault handler
  if(!(pcb->pagetable[page_num] & MEM_PTE_VALID)){
    pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
    if(MemoryPageFaultHandler(pcb) == MEM_FAIL){
      printf("Error: PageFaultHanlder not handled\n");
      return (0);
    }
  }
  //-get and return physical address
  return((pcb->pagetable[page_num] & MEM_PTE_MASK) + page_offset);
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

uint32 MemorySetupPte (uint32 page) {
  return ((page * MEM_PAGESIZE) | MEM_PTE_VALID);
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
  uint32 fault_addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];

  //-grab user stack addr from the currentSavedFrame
  uint32 user_addr = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];

  //-find pagenum for the faulting addr
  uint32 fault_page = fault_addr / MEM_PAGESIZE;
  //-find pagenum for the user stack
  uint32 user_page = user_addr / MEM_PAGESIZE;

  //-segfault (kill the process) if the faulting address is not part of the stack
  //-else allocate a new physical page, setup its PTE, and insert to pagetable
  //-  return MEM_FAIL;
  if(user_page > fault_page){
    printf("SEG FAULT: in PCB %d, fault_addr 0x%08x", GetPidFromAddress(pcb),fault_addr);
    ProcessKill();
    return MEM_FAIL;
  }
  else{
    pcb->pagetable[fault_page] = MemorySetupPte(MemoryAllocPage());
    pcb->npages++;
    return MEM_SUCCESS;
  }

}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {
  int index = 0;
  uint32 freemap_chunk; //int from the freemap array
  uint32 bit_pos = 0;
  uint32 page_num;

  //-return 0 if no free pages
  if(nfreepages == 0){
    printf("MemAllocPg: no free pages\n");
    return 0;
  }

  //-find the available bit in freemap
  while(freemap[index] == 0){
    index++;
  }
  freemap_chunk = freemap[index];
  while((freemap_chunk & 1<<bit_pos) == 0){
    bit_pos++;
  }
  page_num = index * 32 + bit_pos;


  //-set it to unavailable
  MemorySetFreemap(page_num, 0);

  //-decrement number of free pages
  nfreepages--;

  page_refcounters[page_num] =1;

  printf("MemAllocPg: Pg %d allocated by PID: %d\n", page_num, GetCurrentPid());
  //-return this allocated page number
  return page_num;
}

//---------------------------------------------------------------------
// Sets the bit position of the selected page to val(1 or 0)
// to indicate the page is used
//---------------------------------------------------------------------

void MemorySetFreemap(uint32 page, uint32 val){
  uint32 idx = page / 32; //which uint32 it belongs into
  uint32 bit_pos = page % 32;//which bit position in the uint32
  
  //build mask
  uint32 msk = invert(1<<bit_pos);
  freemap[idx] = (freemap[idx] & msk) | (val<<bit_pos);
  return;
}




//---------------------------------------------------------------------
// Sets the bit position of the selected page to 0
// to indicate the page is free
//---------------------------------------------------------------------

void MemoryFreePage(uint32 page) {
  MemorySetFreemap(page, 1);
  nfreepages++;
}

void MemoryFreePte(uint32 pte) {
  uint32 page_num = (pte & MEM_PTE_MASK) / MEM_PAGESIZE;
  if(page_refcounters[page_num]< 1){
    ProcessKill();
  }
  else{
    page_refcounters[page_num]--;
  }

  if(page_refcounters[page_num]==0){
    MemoryFreePage(page_num);
  }

}

int MemoryROPAccessHandler(PCB* pcb){
  int i;

  uint32 new_page;
  // find fault_addr from pcb
  uint32 fault_addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];

  // find l1_page_num for this fault_addr
  uint32 fault_page = fault_addr / MEM_PAGESIZE;

  // find pte associated with this l1_page_num
  // find physical_page_num
  uint32 phys_page = (pcb->pagetable[fault_page] & MEM_PTE_MASK)/MEM_PAGESIZE;
  printf("----ROP Handler called----\n");
  // if refcounter for the physical page < 1
  if(page_refcounters[phys_page] < 1){
    // kill the process, return MEM_FAIL
    ProcessKill();
    return MEM_FAIL;
  }

  // if == 1
  if(page_refcounters[phys_page] == 1){
    // //nobody else is referring to this page
    // mark it as READ/WRITE ← there was a typo here
    pcb->pagetable[fault_page] &= ~MEM_PTE_READONLY;
  }
  else{
    // //other process are referring to this page
    // allocate a new page
    new_page = MemoryAllocPage();
    // copy old data to the new page via “bcopy”
    bcopy((void *)(pcb->pagetable[fault_page]), (void*) (new_page*MEM_PAGESIZE), MEM_PAGESIZE);
    // setup pte for this new page
    pcb->pagetable[fault_page] = MemorySetupPte(new_page);
    // decrement refcounter
    page_refcounters[phys_page]--;
  }

	for (i = 0;i < MEM_L1TABLE_SIZE;++i){
		if (pcb->pagetable[i] & MEM_PTE_VALID){
			printf("\tvirtPg: 0x%x, phys_pg: 0x%x\n", i, pcb->pagetable[i]);
	  }
  }
  printf("----ROP handle exiting----\n");
  return MEM_SUCCESS;
}

void IncreaseRefCount(int page_num){
  page_refcounters[page_num]++; 
}