// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include <stdio.h>
#include "test_util.h"

#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc)
{
  uint64_t aux = *p;
  _yield(); // This makes the race condition highly probable
  aux += inc;
  *p = aux;
}

void my_process_inc(uint64_t argc, char *argv[])
{
  uint64_t n;
  int8_t inc;
  int8_t use_sem;

  if (argc != 3)
  {
    write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 3 argumentos.\n");
    exit_pcs(ERROR);
  }

  if ((n = satoi(argv[0])) <= 0)
  {
    write_out("Error en el satoi de n\n");
    exit_pcs(ERROR);
  }
  if ((inc = satoi(argv[1])) == 0)
  {
    write_out("Error en el satoi de inc \n");
    exit_pcs(ERROR);
  }
  if ((use_sem = satoi(argv[2])) < 0)
  {
    write_out("Error en el satoi de use_sem\n");
    exit_pcs(ERROR);
  }

  if (use_sem)
  {
    write_out("test_sync: Abriendo el semaforo...\n");
    if (_sem_open_init(SEM_ID, 1) == -2)
    { // si devuelve -1 es porq ya si hizo un open, lo cual es verdad y no un error para este test
      write_out("test_sync: ERROR en el sem open init\n");
      exit_pcs(ERROR);
    }
  }

  uint64_t i;
  for (i = 0; i < n; i++)
  {
    if (use_sem)
    {
      if (_sem_wait(SEM_ID) != 0)
      {
        write_out("test_sync: ERROR en el sem wait\n");
        exit_pcs(ERROR);
      }
    }
    slowInc(&global, inc);
    if (use_sem)
    {
      if (_sem_post(SEM_ID) != 0)
      {
        write_out("test_sync: ERROR en el sem post\n");
        exit_pcs(ERROR);
      }
    }
  }

  if (use_sem)
  {
    if (_sem_close(SEM_ID))
    {
      write_out("test_sync: ERROR en el sem close\n");
      exit_pcs(ERROR);
    }
  }

  exit_pcs(EXIT);
}

void test_sync_dummy(int argc, char **argv)
{ //{n, use_sem}
  // n debe ser cuantas veces va a sumar/restar a global, osea un numero mayor a 0
  // use_sem debe ser 0 o 1

  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 2)
  {
    write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 2 argumentos.\n");
    write_out("Enviaste ");
    printDec(argc);
    write_out(" argumentos\n");
    exit_pcs(ERROR);
  }

  write_out("Iniciando test_sync...\n");

  char *argvDec[] = {argv[0], "-1", argv[1], NULL};
  char *argvInc[] = {argv[0], "1", argv[1], NULL};

  global = 0;

  uint64_t i;
  char name[128];
  strcpy(name, "my_process_inc");
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    pids[i] = create_process(&my_process_inc, name, 3, argvDec);
    pids[i + TOTAL_PAIR_PROCESSES] = create_process(&my_process_inc, name, 3, argvInc);
  }

  int pid = _get_pid();
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
  {
    _wait(pids[i], pid);
    _wait(pids[i + TOTAL_PAIR_PROCESSES], pid);
  }

  write_out("Valor final (deberia ser 0 si usaste sem): ");
  printDec(global);
  write_out("\n");

  exit_pcs(EXIT);
}
