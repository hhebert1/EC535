#include "fifo.h"
#include "actor.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main()
{
  fifo_t forkIn;
  fifo_t forkOut1;
  fifo_t forkOut2;

  //begin at fork                                                               
  init_fifo(&forkIn);
  init_fifo(&forkOut1);
  init_fifo(&forkOut2);

//multiply                                                                    
  fifo_t multOut;
  fifo_t multIn;
  init_fifo(&multOut);
  init_fifo(&multIn);

  int i;
  put_fifo(&forkIn, 1); //initialize to 1                                       
  actor_fork(&forkIn, &forkOut1, &forkOut2);

  for(i = 0; i<100; i++)
    {
      put_fifo(&multIn, 42);
      actor_mul(&forkOut1, &multIn, &multOut);
      actor_print(&multOut);
      actor_increment(&forkOut2, &forkIn);
      actor_fork(&forkIn, &forkOut1, &forkOut2);
    }
    return 0;
 }

