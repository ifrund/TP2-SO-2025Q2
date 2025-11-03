#include <stdio.h>
#include "test_util.h"

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  int64_t pid;
  enum State state;
} p_rq;

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

  //write_out("max_processes es ");
  //printDec(max_processes);
  //write_out("\n");
  p_rq p_rqs[max_processes];

  while (1) {

    // Createmax_processes processes
    for (i = 0; i <max_processes; i++) {
      p_rqs[i].pid = create_process(&endless_loop, "endless_loop", 0, argvAux); //aca usamos syscall porq no hay create_pcs dummy 

      if (p_rqs[i].pid == -1) {
        write_out("test_processes: ERROR creating process, no hay mas espacio para procesos\n");
        exit_pcs(ERROR);
      }
      else if(p_rqs[i].pid == -2){
        write_out("test_processes: ERROR creating process, error en alloc\n");
        exit_pcs(ERROR);
      }
      else {
        p_rqs[i].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (i = 0; i <max_processes; i++) {
        action = GetUniform(100) % 2;
        if(p_rqs[i].pid != -1){
          switch (action) {
            case 0:
              if (p_rqs[i].state == RUNNING || p_rqs[i].state == BLOCKED) {
                int argcK = 1;
                char pid_str[21];
                uint_to_str(p_rqs[i].pid, pid_str);
                char *argvK[2];
                argvK[0] = pid_str;
                argvK[1] = NULL;
                if (kill_process(argcK, argvK) == -1) {
                  write_out("test_processes: ERROR killing process\n");
                  exit_pcs(ERROR);
                }
                /*write_out("M");
                char pid_str1[21];
                uint_to_str(p_rqs[i].pid, pid_str1);
                write_out(pid_str1);
                write_out("  ");*/
                p_rqs[i].state = KILLED;
                alive--;
              }
              break;

            case 1:
              if (p_rqs[i].state == RUNNING) {
                int argcB = 1;
                char pid_str[21];
                uint_to_str(p_rqs[i].pid, pid_str);
                char *argvB[2];
                argvB[0] = pid_str;
                argvB[1] = NULL;
                int block = block_process(argcB, argvB);
                if (block == -1) {
                  write_out("test_processes: ERROR blocking process, pid no valido ");
                  printDec(p_rqs[i].pid);
                  write_out(". \n");
                  exit_pcs(ERROR);
                }
                if (block == -2) {
                  write_out("test_processes: ERROR blocking process, estaba ZOMBIE o INVALID ");
                  printDec(p_rqs[i].pid);
                  write_out("\n");
                  exit_pcs(ERROR);
                }
                //write_out("Bloqueamos a ");
                //printDec(p_rqs[i].pid);
                //write_out("\n");
                p_rqs[i].state = BLOCKED;
              }
              break;
          }
        }
      }

      // Randomly unblocks processes
      for (i = 0; i <max_processes; i++)
      if(p_rqs[i].pid != -1){
        if (p_rqs[i].state == BLOCKED && GetUniform(100) % 2) {
          int argcU = 1;
          char pid_str[21];
          uint_to_str(p_rqs[i].pid, pid_str);
          char *argvU[2];
          argvU[0] = pid_str;
          argvU[1] = NULL;
          int ublock = unblock_process(argcU, argvU);
          if (ublock == -1) { //si o si esta bloqueado por el if de arriba
            write_out("test_processes: ERROR unblocking process, pid not valid \n");
            exit_pcs(ERROR);
          }
          if (ublock == -2) { //si o si esta bloqueado por el if de arriba
            write_out("test_processes: ERROR unblocking process, pcs not blocked \n");
            exit_pcs(ERROR);
          }
          //write_out("Desbloqueamos a ");
          //printDec(p_rqs[i].pid);
          //write_out("\n");
          p_rqs[i].state = RUNNING;
        }
      }
    }

    /*ProcessInfo* list = _get_proc_list();
    write_out("\n=== FAKE Lista de procesos ===\n");
    write_out("PID\tNombre\tEstado\tPPID\n");
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
        write_out("\n");
    }

    write_out("-------------------------------------------------------------\n");
    _free(list);*/
  }
  exit_pcs(EXIT);
}

