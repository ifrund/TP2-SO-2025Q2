#ifndef _OURLIB_SO_H_
#define _OURLIB_SO_H_

#include <stdint.h>

#define SECOND_ERROR -2
#define MAX_PCS 64
#define PROCESS_NAME_MAX_LENGTH 32
#define BUFFER_SIZE 128

extern int kill_from_shell;
extern int foreground;
extern int shell_pid;
extern int idle_pid;
extern int bye_shell;

//================================================================================================================================
// SISTEMAS OPERATIVOS
//================================================================================================================================

void argc_1(int argc);

void create_mm();
int alloc(int argc, char **argv);
int free(int argc, char **argv);
int status_count(int argc, char **argv);

int create_process(void *rip, const char *name, int argc, char *argv[]);
int create_process_piped(void *rip, const char *name, int argc, char *argv[], uint64_t *fds);
int kill_process(int argc, char **argv);
void exit_pcs(int ret);
int block_process(int argc, char **argv);
int unblock_process(int argc, char **argv);
int get_proc_list(int argc, char **argv);
int be_nice(int argc, char **argv);

int test_mm(int argc, char **argv);
int test_prio(int argc, char **argv);
int test_pcs(int argc, char **argv);
int test_sync(int argc, char **argv);

int loop(int argc, char **argv);
int wc(int argc, char **argv);
int cat(int argc, char **argv);
int filter(int argc, char **argv);
int mvar(int argc, char **argv);

int sem_open_init(int argc, char **argv);
int sem_wait(int argc, char **argv);
int sem_post(int argc, char **argv);
int sem_close(int argc, char **argv);

#endif