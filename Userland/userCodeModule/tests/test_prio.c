#include <stdint.h>
#include <stdio.h>
#include "test_util.h"

#define MINOR_WAIT_S "1000" 
#define WAIT 21000      //Change this value to make the wait long enough to see theese processes beeing run at least twice
/*

#define LOWEST 0  
#define MEDIUM 1  
#define MEDIUM_2 2
#define MEDIUM_3 3
#define HIGHEST 4
#define TOTAL_PROCESSES 5

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, MEDIUM_2, MEDIUM_3, HIGHEST};

void test_prio_dummy(int argc, char **argv) {
  write_out("ESTE ES EL VIEJO \n");

  if (argc != 0){
    write_out("argc incorrecto\n");
    write_out(PROMPT_START);
    exit_pcs(ERROR);
  }

  int64_t pids[TOTAL_PROCESSES];
  char *argv1[] = {MINOR_WAIT_S, NULL};
  uint64_t i;
  int action;

  write_out("\nCREATING ");
  printDec(TOTAL_PROCESSES);
  write_out(" PROCESSES\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = _create_process(&endless_loop_print, "endless_loop_print", 1, argv1);

  write_out("\nCHANGING PRIORITIES (de algunos)...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++){
      action = GetUniform(100) % 2;
      if(action){ //a algunos los subimos de prio, a otros no xd
            write_out("Cambiando la prio de ");
            printDec(pids[i]);
            write_out(" a la prio: ");
            printDec(prio[i]);
            write_out("\n");
            _be_nice(pids[i], prio[i]);
      }
  }

  char procNamesStorage[MAX_PCS][PROCESS_NAME_MAX_LENGTH] = {0};
  char statusStorage[MAX_PCS][PROCESS_NAME_MAX_LENGTH] = {0};
  char *procNamesArr[MAX_PCS];
  char *statusArr[MAX_PCS];
  uint64_t pidsArr[MAX_PCS];
  uint64_t parentPidsArr[MAX_PCS];
  uint64_t rspsArr[MAX_PCS];

  for (int i = 0; i < MAX_PCS; i++) {
      procNamesArr[i] = procNamesStorage[i];
      statusArr[i] = statusStorage[i];
  }

  _get_proc_list(procNamesArr, pidsArr, parentPidsArr, statusArr, rspsArr);
  write_out("\n=== Lista de procesos ===\n");
  write_out("PID\tNombre\tEstado\tPPID\tRSP\t\n");
  write_out("---------------------------------------------\n");
  for (int i = 0; i < MAX_PCS && procNamesArr[i][0] != '\0'; i++) {
      printDec(pidsArr[i]);
      write_out("\t");
      write_out(procNamesArr[i]);
      write_out("\t");
      write_out(statusArr[i]);
      write_out("\t");
      if (parentPidsArr[i] == (uint64_t)-1) write_out("-1");
      else printDec(parentPidsArr[i]);
      write_out("\t");
      printHex(rspsArr[i]);  
      write_out("\t");
      write_out("\n");
  }

  //bussy_wait(WAIT);
  write_out("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _block_process(pids[i]);

  write_out("CHANGING PRIORITIES (de todos) WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _be_nice(pids[i], prio[i]);

  _get_proc_list(procNamesArr, pidsArr, parentPidsArr, statusArr, rspsArr);
  write_out("\n=== Lista de procesos ===\n");
  write_out("PID\tNombre\tEstado\tPPID\tRSP\t\n");
  write_out("---------------------------------------------\n");
  for (int i = 0; i < MAX_PCS && procNamesArr[i][0] != '\0'; i++) {
      printDec(pidsArr[i]);
      write_out("\t");
      write_out(procNamesArr[i]);
      write_out("\t");
      write_out(statusArr[i]);
      write_out("\t");
      if (parentPidsArr[i] == (uint64_t)-1) write_out("-1");
      else printDec(parentPidsArr[i]);
      write_out("\t");
      printHex(rspsArr[i]);  
      write_out("\t");
      write_out("\n");
  }
 // bussy_wait(WAIT);

  write_out("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _unblock_process(pids[i]);

  _get_proc_list(procNamesArr, pidsArr, parentPidsArr, statusArr, rspsArr);
  write_out("\n=== Lista de procesos ===\n");
  write_out("PID\tNombre\tEstado\tPPID\tRSP\t\n");
  write_out("---------------------------------------------\n");
  for (int i = 0; i < MAX_PCS && procNamesArr[i][0] != '\0'; i++) {
      printDec(pidsArr[i]);
      write_out("\t");
      write_out(procNamesArr[i]);
      write_out("\t");
      write_out(statusArr[i]);
      write_out("\t");
      if (parentPidsArr[i] == (uint64_t)-1) write_out("-1");
      else printDec(parentPidsArr[i]);
      write_out("\t");
      printHex(rspsArr[i]);  
      write_out("\t");
      write_out("\n");
  }
  //bussy_wait(WAIT);

  write_out("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _kill_process(pids[i]);

  write_out(PROMPT_START);
  exit_pcs(EXIT);
}

*/