#ifndef FIFO_H
#define FIFO_H

#define MAXFIFO 1024

typedef struct fifo {
  int data[MAXFIFO];
  unsigned wptr;
  unsigned rptr;
} fifo_t;

void     init_fifo(fifo_t *F);
void     put_fifo (fifo_t *F, int d);
int      get_fifo (fifo_t *F);
unsigned fifo_size(fifo_t *F);

#endif