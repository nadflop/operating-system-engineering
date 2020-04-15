#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here
int MemoryAllocPage(void);
void MemorySetFreemap(uint32, uint32);
void MemoryFreePage(uint32);
void MemoryFreePte(uint32);
uint32 MemorySetupPte(uint32);
int MemoryROPAccessHandler(PCB*);
int malloc(PCB, int);
int mfree(PCB, int);

void increment_refcounter(int);
#endif	// _memory_h_
// #ifndef	_memory_h_
// #define	_memory_h_

// // Put all your #define's in memory_constants.h
// #include "memory_constants.h"

// extern int lastosaddress; // Defined in an assembly file

// //--------------------------------------------------------
// // Existing function prototypes:
// //--------------------------------------------------------

// int MemoryGetSize();
// void MemoryModuleInit();
// uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
// int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
// int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
// int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
// int MemoryPageFaultHandler(PCB *pcb);

// //---------------------------------------------------------
// // Put your function prototypes here
// //---------------------------------------------------------
// // All function prototypes including the malloc and mfree functions go here
// int MemoryAllocPage();
// inline void MemorySetFreemap(int p, int b);
// void MemoryFreePage(uint32 page);
// void MemoryFreePte (uint32 pte);
// uint32 MemorySetupPte (uint32 page);
// void *malloc(); // NEED: how is this setup
// int mfree(); // NEED: how is this setup

// #endif	// _memory_h_