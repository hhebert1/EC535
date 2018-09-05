#include "fifo.h"

#ifndef ACTOR_H
#define ACTOR_H

void actor_mul(fifo_t *i1, fifo_t *i2, fifo_t *q);
void actor_fork(fifo_t *i1, fifo_t *q1, fifo_t *q2);
void actor_increment(fifo_t *i1, fifo_t *q);
void actor_print(fifo_t *i1);

#endif