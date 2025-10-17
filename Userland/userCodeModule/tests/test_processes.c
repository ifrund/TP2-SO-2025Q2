#include <stdio.h>
#include "test_util.h"
#include "../include/userlib.h"

#define MAX_PCS 128
#define PROCESS_NAME_MAX_LENGTH 32

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

    char procNamesStorage[MAX_PCS][PROCESS_NAME_MAX_LENGTH] = {0};
    char statusStorage[MAX_PCS][PROCESS_NAME_MAX_LENGTH] = {0};
    char *procNames[MAX_PCS];
    char *status[MAX_PCS];
    for (int i = 0; i < MAX_PCS; i++) {
        procNames[i] = procNamesStorage[i];
        status[i] = statusStorage[i];
    }

    uint64_t pids[MAX_PCS] = {0};
    uint64_t parentPids[MAX_PCS] = {0};
    uint64_t rsps[MAX_PCS] = {0};

    get_proc_list(procNames, pids, parentPids, status, rsps);

    char buffer[256];

    for (int i = 0; i < MAX_PCS && procNames[i][0] != '\0'; i++) {

      // clear buffer
      buffer[0] = '\0';

      // convert numbers to strings manually
      char pid_str[20], ppid_str[20], rsp_str[20];
      uint_to_str(pids[i], pid_str);
      uint_to_str(parentPids[i], ppid_str);
      uint_to_str(rsps[i], rsp_str);

      strcpy(buffer, "PID:[");
      strcat(buffer, pid_str);
      strcat(buffer, "] | Name:");
      strcat(buffer, procNames[i]);
      strcat(buffer, " | ParentPID=");
      strcat(buffer, ppid_str);
      strcat(buffer, " | RSP=");
      strcat(buffer, rsp_str);
      strcat(buffer, " | Status:");
      strcat(buffer, status[i]);
      strcat(buffer, "\n");
      write_out(buffer);
}

  }
}

