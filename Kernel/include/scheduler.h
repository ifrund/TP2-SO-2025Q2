#ifndef SCHEDULER_H
#define SCHEDULER_H

extern int active_processes;
extern int current_index;

void *scheduling(void *rsp);
void yield();
void last_wish(int pid);
int be_nice(int pid, int new_prio);

#endif