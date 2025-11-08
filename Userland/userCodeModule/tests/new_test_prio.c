#include <stdint.h>
#include <stdio.h>
#include "test_util.h"
#include "../include/userlib.h"

#define HIGHEST 0
#define MEDIUM 1  
#define MEDIUM_2 2
#define MEDIUM_3 3
#define LOWEST 4  

#define TOTAL_PROCESSES 5

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM_3, MEDIUM_2, MEDIUM, HIGHEST};

uint64_t max_value = 0;

void zero_to_max() {
  uint64_t value = 0;

  while (value++ != max_value); //si el while tarda menos q 5 ticks no sirve de nada este test xd
  //con 15.000.000 tarda más de 5 ticks asegurado

  write_out("PROCESS ");
  printDec(_get_pid());
  write_out(" DONE! -- ");
  exit_pcs(EXIT);
}

uint64_t test_prio_new(uint64_t argc, char *argv[]) {

    int64_t pids[TOTAL_PROCESSES];
    char *ztm_argv[] = {0};
    uint64_t i;
    static char *pcs_name = "zero_to_max";
    int my_pid = _get_pid();

    if (argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 argumento.\n");
        write_out(PROMPT_START);
        exit_pcs(ERROR);
    }


    if ((max_value = satoi(argv[0])) <= 0){
        write_out("argv:");
        write_out(argv[0]);
        write_out("\n");
        write_out("error en el satoi\n");
        write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    write_out("SAME PRIORITY...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++){
        pids[i] = create_process(&zero_to_max, pcs_name, 0, ztm_argv);
        write_out("CREATED: ");
        printDec(pids[i]);
        write_out("  ");
    }
    write_out("\n");

    // Expect to see them finish at the same time
    //write_out("Primer wait\n");
    for (i = 0; i < TOTAL_PROCESSES; i++){
        _wait(pids[i], my_pid, pcs_name);
    }

    write_out("\nSAME PRIORITY, THEN CHANGE IT...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = create_process(&zero_to_max, pcs_name, 0, ztm_argv);
        _be_nice(pids[i], prio[i]);
        write_out("PROCESS ");
        printDec(pids[i]);
        write_out(" NEW PRIORITY:");
        printDec(prio[i]);
        write_out("  ");
    }
    write_out("\n");
    // Expect the priorities to take effect

    //write_out("Segundo wait\n");
    for (i = 0; i < TOTAL_PROCESSES; i++){
        _wait(pids[i], my_pid, pcs_name);
    }

    write_out("\nSAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = create_process(&zero_to_max, pcs_name, 0, ztm_argv);
        _block_process(pids[i]);
        _be_nice(pids[i], prio[i]);
        write_out("PROCESS ");
        printDec(pids[i]);
        write_out(" NEW PRIORITY:");
        printDec(prio[i]);
        write_out("  ");
    }

    for (i = 0; i < TOTAL_PROCESSES; i++){
        _unblock_process(pids[i]);
    }

    // Expect the priorities to take effect

    //write_out("Tercer wait\n");
    for (i = 0; i < TOTAL_PROCESSES; i++){
        _wait(pids[i], my_pid, pcs_name);
    }


    write_out("\nTermino el test prio ;) \n");
    exit_pcs(EXIT);
    return 0;
}