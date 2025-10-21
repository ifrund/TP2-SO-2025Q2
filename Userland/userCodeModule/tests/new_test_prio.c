#include <stdint.h>
#include <stdio.h>
#include "test_util.h"

char PROMPT_START2[] = {127, 0};
#define LOWEST 0  
#define MEDIUM 1  
#define MEDIUM_2 2
#define MEDIUM_3 3
#define HIGHEST 4
#define TOTAL_PROCESSES 5

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, MEDIUM_2, MEDIUM_3, HIGHEST};

uint64_t max_value = 0;

void zero_to_max() {
  uint64_t value = 0;

  while (value++ != max_value);

  write_out("PROCESS ");
  printDec(_get_pid());
  write_out(" DONE!\n");
}

uint64_t test_prio_new(uint64_t argc, char *argv[]) {

    int64_t pids[TOTAL_PROCESSES];
    char *ztm_argv[] = {0};
    uint64_t i;

    if (argc != 1){
        write_out("argc incorrecto\n");
        write_out(PROMPT_START2);
        exit_pcs(ERROR);
    }


    if ((max_value = satoi(argv[0])) <= 0){
        write_out("argv:");
        write_out(argv[0]);
        write_out("\n");
        write_out("error en el satoi\n");
        write_out(PROMPT_START2);
        exit_pcs(ERROR);
    }

    write_out("SAME PRIORITY...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        pids[i] = _create_process(&zero_to_max, "zero_to_max", 0, ztm_argv);

    // Expect to see them finish at the same time

    for (i = 0; i < TOTAL_PROCESSES; i++)
        //_wait(pids[i]);

    write_out("SAME PRIORITY, THEN CHANGE IT...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = _create_process(&zero_to_max, "zero_to_max", 0, ztm_argv);
        _be_nice(pids[i], prio[i]);
        write_out("  PROCESS ");
        printDec(pids[i]);
        write_out(" NEW PRIORITY:");
        printDec(prio[i]);
        write_out("\n");
    }

    // Expect the priorities to take effect

    for (i = 0; i < TOTAL_PROCESSES; i++)
        //_wait(pids[i]);

    write_out("SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = _create_process(&zero_to_max, "zero_to_max", 0, ztm_argv);
        _block_process(pids[i]);
        _be_nice(pids[i], prio[i]);
        write_out("  PROCESS ");
        printDec(pids[i]);
        write_out(" NEW PRIORITY:");
        printDec(prio[i]);
        write_out("\n");
    }

    for (i = 0; i < TOTAL_PROCESSES; i++)
        _unblock_process(pids[i]);

    // Expect the priorities to take effect

    for (i = 0; i < TOTAL_PROCESSES; i++)
        //_wait(pids[i]);


    write_out("Termino el test prio ;) \n");
    write_out(PROMPT_START2);
    exit_pcs(EXIT);
    return 0;
}