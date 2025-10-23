#include <stdio.h>
#include "test_util.h"

char PROMPT_START3[] = {127, 0};

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  int64_t pid;
  enum State state;
} p_rq;

void uint_to_str(uint64_t num, char *out) {
    char temp[20];
    int i = 0;

    if (num == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while (num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (int j = 0; j < i; j++)
        out[j] = temp[i - j - 1];
    out[i] = '\0';
}

// append src to dest
void strcat(char *dest, const char *src) {
    while (*dest) dest++;  // move to end of dest
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

//recibe un int, que es la cantidad max de pcs
void test_processes_dummy(int argc, char **argv) {

  uint8_t i;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes=0;
  char *argvAux[] = {0};

  if (argc != 1){
    write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 argumento.\n");
    exit_pcs(ERROR);
  }

  if ((max_processes = satoi(argv[0])) <= 0){
    write_out("error en el satoi\n");
    exit_pcs(ERROR);
  }

  write_out("max_processes es ");
  printDec(max_processes);
  write_out("\n");
  p_rq p_rqs[max_processes];

  while (1) {

    // Createmax_processes processes
    for (i = 0; i <max_processes; i++) {
      p_rqs[i].pid = _create_process(&endless_loop, "endless_loop", 0, argvAux);

      if (p_rqs[i].pid == -1) {
        write_out("test_processes: ERROR creating process\n");
        exit_pcs(ERROR);
      } else {
        p_rqs[i].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (i = 0; i <max_processes; i++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[i].state == RUNNING || p_rqs[i].state == BLOCKED) {
              if (_kill_process(p_rqs[i].pid) == -1) {
                write_out("test_processes: ERROR killing process\n");
                exit_pcs(ERROR);
              }
              write_out("Matamos a ");
              printDec(p_rqs[i].pid);
              write_out("\n");
              p_rqs[i].state = KILLED;
              alive--;
            }
            break;

          case 1:
            if (p_rqs[i].state == RUNNING) {
              int ret = _block_process(p_rqs[i].pid);
              if (ret == -1) {
                write_out("test_processes: ERROR blocking process, pid no existe ");
                printDec(p_rqs[i].pid);
                write_out(". \n");
                exit_pcs(ERROR);
              }
              write_out("Bloqueamos a ");
              printDec(p_rqs[i].pid);
              write_out("\n");
              p_rqs[i].state = BLOCKED;
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (i = 0; i <max_processes; i++)
        if (p_rqs[i].state == BLOCKED && GetUniform(100) % 2) {
          if (_unblock_process(p_rqs[i].pid) == -1) { //si o si esta bloqueado por el if de arriba
            write_out("test_processes: ERROR unblocking process\n");
            exit_pcs(ERROR);
          }
          write_out("Desbloqueamos a ");
          printDec(p_rqs[i].pid);
          write_out("\n");
          p_rqs[i].state = RUNNING;
        }
    }

    ProcessInfo* list = _get_proc_list();
    write_out("\n=== Lista de procesos ===\n");
    write_out("PID\tNombre\tEstado\tPPID\tRSP\tPrio\tChilds\tFD\n");
    write_out("-------------------------------------------------------------\n");

    //iterar sobre la lista
    for (int i = 0; i < MAX_PCS; i++) {
        ProcessInfo* p = &list[i];
        if (p->pid == -1)  // Slot vacío
            continue;

        printDec(p->pid);
        write_out("\t");
        write_out(p->name);
        write_out("\t");
        write_out(p->state);
        write_out("\t");
        if (p->parentPid == (uint64_t)-1) write_out("-1");
        else printDec(p->parentPid);
        write_out("\t0x");
        printHex((uint64_t)p->rsp); 
        write_out("\t");
        write_out(p->my_prio);
        write_out("\t");
        printDec(p->childrenAmount);
        write_out("\t");
        printDec(p->fileDescriptorCount);
        write_out("\n");
    }

    write_out("-------------------------------------------------------------\n");

    _free(list);
    write_out(PROMPT_START3);
    exit_pcs(EXIT);

  }
}

