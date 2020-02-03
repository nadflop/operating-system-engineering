#include "usertraps.h"

void main (int x)
{
  Printf("Hello World!\n");
  //printf("Current PID: %d\n", Getpid());
  Getpid();
  while(1); // Use CTRL-C to exit the simulator
}
