#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>
#include "proc.h"
#include "lib.h"
#include "interrupts.h"

extern int process_count;
extern int current_index;

void * scheduling(void *rsp);
void yield();
int be_nice(int pid, int new_prio);

#endif