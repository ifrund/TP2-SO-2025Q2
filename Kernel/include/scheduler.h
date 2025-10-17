#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>
#include "proc.h"
#include "lib.h"
#include "interrupts.h"

void * scheduling(void *rsp);
void yield();
int be_nice(int pid);

#endif