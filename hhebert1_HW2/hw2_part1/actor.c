#include "actor.h"
#include "fifo.h"
#include <assert.h>
#include <stdio.h>

void actor_mul(fifo_t *i1, fifo_t *i2, fifo_t *q)
{
  assert(i1!=0);
  assert(i2!=0);
  assert(q !=0);
  // firing rule: fire if there's at least one token on each                    
  // of the inputs, i1 and i2.                                                  
  if ((fifo_size(i1) > 0) && (fifo_size(i2) > 0))
    // action: read a token from each input,                                    
    // multiply them together, write the result to the output queue             
    put_fifo(q, get_fifo(i1) * get_fifo(i2));
}

void actor_increment(fifo_t *i1, fifo_t *q)
{
  assert(i1!=0);
  assert(q !=0);
  if(fifo_size(i1) > 0)
    put_fifo(q, get_fifo(i1) + 1);
}

void actor_print(fifo_t *i1)
{
  assert(i1!=0);
  if (fifo_size(i1) > 0)
    printf("%d\n" , get_fifo(i1));
}

void actor_fork(fifo_t *i1, fifo_t *q1, fifo_t *q2)
{
  assert(i1!=0);
  assert(q1!=0);
  assert(q2!=0);

  if(fifo_size(i1) > 0)
    {
   int d = get_fifo(i1);

    put_fifo(q1, d);
    put_fifo(q2, d);
}
}