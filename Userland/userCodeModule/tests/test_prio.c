#include <stdint.h>
#include <stdio.h>
#include "test_util.h"

#define MINOR_WAIT 1000000 // TODO: Change this value to prevent a process from flooding the screen
#define WAIT 10000000      // TODO: Change this value to make the wait long enough to see theese processes beeing run at least twice

#define TOTAL_PROCESSES 3
#define LOWEST 0  // TODO: Change as required
#define MEDIUM 1  // TODO: Change as required
#define HIGHEST 2 // TODO: Change as required

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

void test_prio_dummy(int argc, char **argv) {
  
  if (argc != 0){
    write_out("argc incorrecto\n");
    exit_pcs(ERROR);
  }

  int64_t pids[TOTAL_PROCESSES];
  //char *argv[] = {0};
  uint64_t i;

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = _create_process(&endless_loop_print, "endless_loop_print", 0, argv);

  bussy_wait(WAIT);
  write_out("\nCHANGING PRIORITIES...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _be_nice(pids[i]);

  bussy_wait(WAIT);
  write_out("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _block_process(pids[i]);

  write_out("CHANGING PRIORITIES WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _be_nice(pids[i]);

  write_out("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _unblock_process(pids[i]);

  bussy_wait(WAIT);
  write_out("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _kill_process(pids[i]);

  
  exit_pcs(EXIT);
}
