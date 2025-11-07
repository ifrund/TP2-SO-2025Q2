#ifndef _OURLIB_SO_H_
#define _OURLIB_SO_H_

#include "userlib.h"

#define SECOND_ERROR -2
#define MAX_PCS 64
#define PROCESS_NAME_MAX_LENGTH 32
#define BUFFER_SIZE 128

extern int foreground;
extern int shell_pid;
extern int idle_pid;

//================================================================================================================================
// SISTEMAS OPERATIVOS
//================================================================================================================================

void argc_1(int argc);

int create_process(void * rip, const char *name, int argc, char *argv[]);
int create_process_piped(void * rip, const char *name, int argc, char *argv[], uint64_t* fds);
void kill(int argc, char ** argv);
void exit_pcs(int ret);
void block_process(int argc, char ** argv);
void unblock_process(int argc, char ** argv);
void get_proc_list(int argc, char ** argv);
int get_pid();
void yield(int argc, char ** argv);
void be_nice(int argc, char ** argv);
void status_count(int argc, char** argv);

void loop(int argc, char **argv);
void wc(int argc, char **argv);
void cat(int argc, char **argv);
void filter(int argc, char **argv);
void mvar(int argc, char ** argv);
void msg(int argc, char ** argv);

void sem_open_init(int argc, char ** argv);
void sem_wait(int argc, char ** argv);
void sem_post(int argc, char ** argv);
void sem_close(int argc, char ** argv);

#endif