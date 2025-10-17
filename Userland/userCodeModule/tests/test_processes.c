#include <stdio.h>
#include "test_util.h"
#include "../include/userlib.h"

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  int64_t pid;
  enum State state;
} p_rq;

//recibe un int, que es la cantidad max de pcs
int64_t test_processes(uint64_t argc, char *argv[]) {

  uint8_t i;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes=0;
  char *argvAux[] = {0};

  if (argc != 1){
    write_out("argc incorrecto\n");
    return -1;
  }

  if ((max_processes = satoi(argv[0])) <= 0){
    write_out("error en el satoi\n");
    return -1;
  }

  p_rq p_rqs[max_processes];

  while (1) {

    // Create max_processes processes
    for (i = 0; i < max_processes; i++) {
      p_rqs[i].pid = create_process(&endless_loop, "endless_loop", 0, argvAux);

      if (p_rqs[i].pid == -1) {
        write_out("test_processes: ERROR creating process\n");
        return -1;
      } else {
        p_rqs[i].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (i = 0; i < max_processes; i++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[i].state == RUNNING || p_rqs[i].state == BLOCKED) {
              if (kill_process(p_rqs[i].pid) == -1) {
                write_out("test_processes: ERROR killing process\n");
                return -1;
              }
              p_rqs[i].state = KILLED;
              alive--;
            }
            break;

          case 1:
            if (p_rqs[i].state == RUNNING) {
              if (block_process(p_rqs[i].pid) == -1) {
                write_out("test_processes: ERROR blocking process\n");
                return -1;
              }
              p_rqs[i].state = BLOCKED;
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (i = 0; i < max_processes; i++)
        if (p_rqs[i].state == BLOCKED && GetUniform(100) % 2) {
          if (unblock_process(p_rqs[i].pid) == -1) {
            write_out("test_processes: ERROR unblocking process\n");
            return -1;
          }
          p_rqs[i].state = RUNNING;
        }
    }
  }
}

