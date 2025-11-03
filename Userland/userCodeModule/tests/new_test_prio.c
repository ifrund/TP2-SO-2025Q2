#include <stdint.h>
#include <stdio.h>
#include "test_util.h"

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
        pids[i] = create_process(&zero_to_max, "zero_to_max", 0, ztm_argv);
        write_out("CREATED: ");
        printDec(pids[i]);
        write_out("  ");
    }
    write_out("\n");

    // Expect to see them finish at the same time
    write_out("Primer wait\n");
    char name[128];
    strcpy(name, "zero_to_max");
    for (i = 0; i < TOTAL_PROCESSES; i++){
        static char wpid_str[21];     
        uint_to_str(pids[i], wpid_str);

        static char *new_argv[3];  
        new_argv[0] = wpid_str;
        new_argv[1] = name;
        new_argv[2] = NULL;
        wait(2, new_argv);
    }

    write_out("\nSAME PRIORITY, THEN CHANGE IT...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = create_process(&zero_to_max, "zero_to_max", 0, ztm_argv);
        static char pid_str[21];   
        static char prio_str[21];
        uint_to_str(pids[i], pid_str);
        uint_to_str(prio[i], prio_str);
        static char *new_argv[3];  
        new_argv[0] = pid_str;
        new_argv[1] = prio_str;
        new_argv[2] = NULL;
        be_nice(2, new_argv);
        write_out("PROCESS ");
        printDec(pids[i]);
        write_out(" NEW PRIORITY:");
        printDec(prio[i]);
        write_out("  ");
    }
    write_out("\n");
    // Expect the priorities to take effect

    write_out("Segundo wait\n");
    for (i = 0; i < TOTAL_PROCESSES; i++){
        static char wpid_str[21];     
        uint_to_str(pids[i], wpid_str);

        static char *new_argv[3];  
        new_argv[0] = wpid_str;
        new_argv[1] = name;
        new_argv[2] = NULL;
        wait(2, new_argv);
    }

    write_out("\nSAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = create_process(&zero_to_max, "zero_to_max", 0, ztm_argv);
        static char pid_str[21];
        uint_to_str(pids[i], pid_str);
        static char* new_argv[1];
        new_argv[0]=pid_str;
        block_process(1, new_argv);
        static char pid_str2[21];   
        static char prio_str[21];
        uint_to_str(pids[i], pid_str2);
        uint_to_str(prio[i], prio_str);
        static char *new_argv2[3];  
        new_argv2[0] = pid_str2;
        new_argv2[1] = prio_str;
        new_argv2[2] = NULL;
        be_nice(2, new_argv2);
        write_out("PROCESS ");
        printDec(pids[i]);
        write_out(" NEW PRIORITY:");
        printDec(prio[i]);
        write_out("  ");
    }

    for (i = 0; i < TOTAL_PROCESSES; i++){
        static char pid_str[21];
        uint_to_str(pids[i], pid_str);
        static char* new_argv[1];
        new_argv[0]=pid_str;
        unblock_process(1, new_argv);
        //_unblock_process(pids[i]);
    }

    // Expect the priorities to take effect

    write_out("Tercer wait\n");
    for (i = 0; i < TOTAL_PROCESSES; i++){
        static char wpid_str[21];     
        uint_to_str(pids[i], wpid_str);

        static char *new_argv[3];  
        new_argv[0] = wpid_str;
        new_argv[2] = NULL;
        new_argv[1] = name;
        wait(2, new_argv);
    }


    write_out("\nTermino el test prio ;) \n");
    write_out(PROMPT_START);
    exit_pcs(EXIT);
    return 0;
}