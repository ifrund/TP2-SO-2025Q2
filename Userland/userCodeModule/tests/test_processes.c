// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdio.h>
#include "test_util.h"

enum State
{
  RUNNING,
  BLOCKED,
  KILLED
};

typedef struct P_rq
{
  int64_t pid;
  enum State state;
} p_rq;

// append src to dest
void strcat(char *dest, const char *src)
{
  while (*dest)
    dest++; // move to end of dest
  while (*src)
  {
    *dest++ = *src++;
  }
  *dest = '\0';
}

// recibe un int, que es la cantidad max de pcs
void test_processes_dummy(int argc, char **argv)
{

  uint8_t i;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes = 0;
  char *argvAux[] = {0};

  if (argc != 1)
  {
    write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 argumento.\n");
    exit_pcs(ERROR);
  }

  if ((max_processes = satoi(argv[0])) <= 0)
  {
    write_out("error en el satoi\n");
    exit_pcs(ERROR);
  }

  p_rq p_rqs[max_processes];
  int my_pid = _get_pid();
  char *name = "endless_loop";
  while (1)
  {

    // Creamos max procesos
    for (i = 0; i < max_processes; i++)
    {
      p_rqs[i].pid = create_process(&endless_loop, name, 0, argvAux); // usamos syscall porque no hay create_pcs dummy

      if (p_rqs[i].pid == -1)
      {
        write_out("test_processes: ERROR creando proceso, no hay mas espacio para procesos\n");
        exit_pcs(ERROR);
      }
      else if (p_rqs[i].pid == -2)
      {
        write_out("test_processes: ERROR creando proceso, error en alloc\n");
        exit_pcs(ERROR);
      }
      else
      {
        p_rqs[i].state = RUNNING;
        alive++;
      }
    }

    // Aleatoriamente mata, bloquea y desbloquea procesos hasta que todos esten muertos
    while (alive > 0)
    {

      for (i = 0; i < max_processes; i++)
      {
        action = GetUniform(100) % 2;
        switch (action)
        {
        case 0:
          if (p_rqs[i].state == RUNNING || p_rqs[i].state == BLOCKED)
          {
            if (_kill_process(p_rqs[i].pid) == -1)
            {
              write_out("test_processes: ERROR matando proceso\n");
              exit_pcs(ERROR);
            }
            p_rqs[i].state = KILLED;
            alive--;
          }
          break;

        case 1:
          if (p_rqs[i].state == RUNNING)
          {
            int block = _block_process(p_rqs[i].pid);
            if (block == -1)
            {
              write_out("test_processes: ERROR bloqueando proceso, pid no valido ");
              printDec(p_rqs[i].pid);
              write_out(". \n");
              exit_pcs(ERROR);
            }
            if (block == -2)
            {
              write_out("test_processes: ERROR bloqueando proceso, estaba ZOMBIE o INVALID ");
              printDec(p_rqs[i].pid);
              write_out("\n");
              exit_pcs(ERROR);
            }
            p_rqs[i].state = BLOCKED;
          }
          break;
        }
      }

      // Desbloquea procesos aleatoriamente
      for (i = 0; i < max_processes; i++)
        if (p_rqs[i].state == BLOCKED && GetUniform(100) % 2)
        {
          int ublock = _unblock_process(p_rqs[i].pid);
          if (ublock == -1)
          { // si o si esta bloqueado por el if de arriba
            write_out("test_processes: ERROR desbloqueando proceso, pid no valido \n");
            exit_pcs(ERROR);
          }
          if (ublock == -2)
          { // si o si esta bloqueado por el if de arriba
            write_out("test_processes: ERROR desbloqueando proceso, proc. no bloqueado \n");
            exit_pcs(ERROR);
          }
          p_rqs[i].state = RUNNING;
        }
    }

    // un wait por si creamos muchos procesos
    // así no se maxea nuestro array de procesos
    for (i = 0; i < max_processes; i++)
    {
      if (p_rqs[i].pid > 0)
      {
        // Esperamos a que termine cada proceso que creamos
        int ret = _wait(p_rqs[i].pid, my_pid);
        if (ret == -1)
        {
          write_out("test_processes: ERROR esperando proceso, pid no valido ");
          printDec(p_rqs[i].pid);
          write_out("\n");
        }
      }
    }
  }
  exit_pcs(EXIT);
}
