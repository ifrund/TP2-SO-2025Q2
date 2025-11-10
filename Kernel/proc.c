// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include <stddef.h>
#include "include/lib_asm.h"
#include "include/lib_mem.h"
#include "include/lib_str.h"
#include "include/memory_manager.h"
#include "include/proc.h"
#include "include/pipes.h"
#include "include/scheduler.h"

PCB *process_table[MAX_PCS] = {NULL};
int IDLE_PID;
int SHELL_PID;

// tabla de procesos
int create_process(void *rip, char *name, int argc, char *argv[], uint64_t *fds)
{
    int i, my_pid = -1;

    for (i = 0; i < MAX_PCS; i++)
    {
        if (process_table[i] == NULL)
        {
            my_pid = i;
            break;
        }
        if (process_table[i]->state == ZOMBIE)
        {
            PCB *zombie = process_table[i];

            if (zombie->argv != NULL)
            {
                for (int j = 0; zombie->argv[j] != NULL; j++)
                {
                    free(zombie->argv[j]);
                }
                free(zombie->argv);
            }

            free(zombie->stack_base);
            free(zombie);
            process_table[i] = NULL;
            my_pid = i;
            break;
        }
    }
    // de esta manera el pid simepre es el indice en la tabla

    if (my_pid == -1)
    {
        return -1;
    }

    // Inicializamos PCB:
    PCB *pcb = alloc(sizeof(PCB));
    if (pcb == NULL)
        return -2;
    process_table[my_pid] = pcb;

    // Informacion general
    memset(pcb, 0, sizeof(PCB));

    // Reservamos espacio para el stack
    void *stack_base = alloc(MAX_STACK_SIZE);
    if (stack_base == NULL)
    {
        free(pcb);
        process_table[i] = NULL;
        return -2;
    }
    pcb->stack_base = stack_base;

    // Preparamos argv
    char **argv_copy = NULL;
    if (argc > 0 && argv != NULL)
    {
        argv_copy = alloc(sizeof(char *) * (argc + 1)); // +1 para NULL final
        if (argv_copy == NULL)
        {
            free(stack_base);
            free(process_table[i]);
            process_table[i] = NULL;
            return -2;
        }

        for (int k = 0; k < argc; k++)
        {
            int len = strlen(argv[k]) + 1;
            argv_copy[k] = alloc(len);
            if (argv_copy[k] == NULL)
            {
                // rollback en caso de error
                for (int m = 0; m < k; m++)
                    free(argv_copy[m]);
                free(argv_copy);
                free(stack_base);
                free(pcb);
                process_table[i] = NULL;
                return -2;
            }
            memcpy(argv_copy[k], argv[k], len);
        }
        argv_copy[argc] = NULL;
    }
    process_table[my_pid]->argv = argv_copy;

    // Stack
    pcb->rsp = stack_base + MAX_STACK_SIZE;
    pcb->rsp = _create_stack(pcb->rsp, rip, argc, argv_copy);

    // Informacion
    memset(pcb->name, 0, PROCESS_NAME_MAX_LENGTH);
    memcpy(pcb->name, name, PROCESS_NAME_MAX_LENGTH - 1);
    pcb->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';
    pcb->PID = my_pid;
    pcb->parent_pid = get_pid();

    pcb->time_used = 0;
    if (strcmp(name, "idle") == 0)
    {
        pcb->my_max_time = IDLE_Q;
        pcb->my_prio = LEVEL_IDLE;
        // No lo dejamos ni en ready, eso se hara en el sch cuando se necesite
        pcb->state = BLOCKED;
    }
    else
    {
        pcb->my_max_time = QUANTUM;
        pcb->my_prio = LEVEL_4;
        pcb->state = READY;
    }

    // Informacion de los hijos
    pcb->child_amount = 0;
    for (int i = 0; i < MAX_PCS; i++)
    {
        pcb->childs[i] = -1;
    }

    // Registrar como hijo del padre
    int parent_pid = pcb->parent_pid;
    if (parent_pid >= 0 && parent_pid < MAX_PCS && process_table[parent_pid] != NULL)
    {
        PCB *parent = process_table[parent_pid];
        for (int i = 0; i < MAX_PCS; i++)
        {
            if (parent->childs[i] == -1)
            {
                parent->childs[i] = my_pid;
                parent->child_amount++;
                break;
            }
        }
    }
    pcb->blocks_amount = 0;

    if (fds == NULL)
    {
        // Comportamiento por default: STDIN=0, STDOUT=1, STDERR=2
        pcb->file_descriptors[0] = 0; // STDIN
        pcb->file_descriptors[1] = 1; // STDOUT
        pcb->file_descriptors[2] = 2; // STDERR
    }
    else
    {
        // Comportamiento con pipe: Copia los FDs del array
        pcb->file_descriptors[0] = fds[0]; // STDIN
        pcb->file_descriptors[1] = fds[1]; // STDOUT
        // si los FDs apuntan a pipes , registrar la apertura
        if (fds[0] > 2)
            pipe_register((int)fds[0], PIPE_READ_END);
        if (fds[1] > 2)
            pipe_register((int)fds[1], PIPE_WRITE_END);
        pcb->file_descriptors[2] = 2; // STDERR
    }
    pcb->yielding = 0;
    pcb->total_ticks = 0;
    pcb->changes = 0;
    pcb->yield_changes = 0;
    pcb->dad_blocked = 0;

    if (strcmp(name, "wait") == 0)
    {
        active_processes++;
        yield(); // necesitamos q el pcs q crea un wait deje sus quehaceres y se frene
        return pcb->PID;
    }

    active_processes++;
    return pcb->PID;
}

int block_process(int pid)
{
    if (!is_pid_valid(pid))
        return -1;

    process_state state = process_table[pid]->state;
    if (state == READY || state == RUNNING)
    {
        process_table[pid]->state = BLOCKED;
        process_table[pid]->blocks_amount++;
        yield();
        return 0;
    }
    if (state == BLOCKED)
    {
        process_table[pid]->blocks_amount++;
        yield();
        return 0;
    }
    else
    { // si entra aca es ZOMBIE o INVALID
        return -2;
    }
}

int unblock_process(uint64_t pid)
{
    if (!is_pid_valid(pid))
        return -1;

    if (process_table[pid]->state != BLOCKED)
        return -2;

    if (process_table[pid]->blocks_amount > 1)
    { // si hubo varios blocks, los descontamos
        process_table[pid]->blocks_amount--;
        return 0;
    }
    // si estamos aca, significa q solo hubo un block, asiq lo descontamos y liberamos el pcs
    process_table[pid]->blocks_amount--;
    process_table[pid]->state = READY;

    return 0;
}

void kill_pipes(PCB *proc)
{
    for (int i = 0; i < MAX_FD; i++)
    {
        int fd_real = proc->file_descriptors[i];
        if (fd_real > 2)
        {
            // FD 0 (STDIN) (extremo de lectura de un pipe)
            if (i == 0)
            {
                pipe_close(fd_real, PIPE_READ_END);
            }
            // FD 1 (STDOUT) (extremo de escritura de un pipe)
            else if (i == 1)
            {
                pipe_close(fd_real, PIPE_WRITE_END);
            }
            // Otros FD: tratamos de cerrar ambos extremos
            else
            {
                pipe_close(fd_real, PIPE_READ_END);
                pipe_close(fd_real, PIPE_WRITE_END);
            }
        }
    }
}

void kill_all_of_type(const char *target_name)
{
    for (int i = 0; i < MAX_PCS; i++)
    {
        if (process_table[i] && strcmp(process_table[i]->name, target_name) == 0 && process_table[i]->state != ZOMBIE)
        {
            kill_process(process_table[i]->PID);
        }
    }
}

void kill_mvar(PCB *proc)
{
    // ciclamos en process_table[] para ver si quedan readers/writers
    int amount = 0;
    for (int i = 0; i < MAX_PCS && amount == 0; i++)
    {
        if (process_table[i] && strcmp(proc->name, process_table[i]->name) == 0 && process_table[i]->state != ZOMBIE)
        {
            amount++;
        }
    }

    if (amount > 0)
        return;

    // soy el último reader/writer tengo q cerrar los pipes
    kill_pipes(proc);

    // y matar a los q queden del otro grupo
    if (strcmp(proc->name, "mvar_writer") == 0)
    {
        kill_all_of_type("mvar_reader");
    }
    else if (strcmp(proc->name, "mvar_reader") == 0)
    {
        kill_all_of_type("mvar_writer");
    }
}

int kill_process(uint64_t pid)
{
    if (!is_pid_valid(pid))
        return -1;

    PCB *proc = process_table[pid];
    if (proc->state == ZOMBIE)
    {
        return -2;
    }
    process_table[pid]->state = ZOMBIE;

    if (strcmp("mvar_writer", proc->name) == 0 || strcmp("mvar_reader", proc->name) == 0)
    {
        kill_mvar(proc);
    }
    else
    {
        kill_pipes(proc);
    }

    uint64_t parent_pid = proc->parent_pid;

    if (parent_pid < MAX_PCS && process_table[parent_pid] != NULL)
    {

        // Despertar al padre
        if(proc->dad_blocked){
            unblock_process(parent_pid);
        }

        // me elimino de la lista de hijos del padre
        PCB *parent = process_table[parent_pid];
        for (int i = 0; i < MAX_PCS; i++)
        {
            if (parent->childs[i] == pid)
            {
                parent->childs[i] = -1;
                parent->child_amount--;
                break;
            }
        }
    }

    // a todos mis hijos se los dejo a idle, no improta q este bloqueado
    PCB *idle = process_table[IDLE_PID];
    for (int i = 0; i < proc->child_amount; i++)
    {
        int childPid = proc->childs[i];
        PCB *child = process_table[childPid];
        child->parent_pid = IDLE_PID;
        idle->childs[idle->child_amount++] = childPid;
    }
    proc->child_amount = 0; // dejo esto en 0 por si sigo apareciendo en el ps y q se vea lindo :)

    active_processes--;
    last_wish(pid); // yield especial porque este pid ya es zombie
    return 0;
}

ProcessInfo *get_proc_list()
{
    ProcessInfo *list = alloc(sizeof(ProcessInfo) * MAX_PCS);
    if (list == NULL)
        return NULL;

    for (int i = 0; i < MAX_PCS; i++)
    {
        PCB *p = process_table[i];
        ProcessInfo *info = &list[i];

        if (p == NULL)
        {
            // Proceso vacio
            info->pid = -1;
            strncpy(info->state, "INVALID", 15);
            continue;
        }

        // General
        memcpy(info->name, p->name, PROCESS_NAME_MAX_LENGTH - 1);
        info->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0';

        info->pid = p->PID;
        info->parent_pid = p->parent_pid;

        switch (p->state)
        {
        case RUNNING:
            strncpy(info->state, "RUNNING", INFO_STR_LENGTH);
            break;
        case BLOCKED:
            strncpy(info->state, "BLOCKED", INFO_STR_LENGTH);
            break;
        case READY:
            strncpy(info->state, "READY", INFO_STR_LENGTH);
            break;
        case ZOMBIE:
            strncpy(info->state, "ZOMBIE", INFO_STR_LENGTH);
            break;
        default:
            strncpy(info->state, "UNKNOWN", INFO_STR_LENGTH);
        }

        info->state[15] = '\0';
        info->rsp = (uint64_t)p->rsp;

        switch (p->my_prio)
        {
        case LEVEL_0:
            strncpy(info->my_prio, "LEVEL_0", INFO_STR_LENGTH);
            break;
        case LEVEL_1:
            strncpy(info->my_prio, "LEVEL_1", INFO_STR_LENGTH);
            break;
        case LEVEL_2:
            strncpy(info->my_prio, "LEVEL_2", INFO_STR_LENGTH);
            break;
        case LEVEL_3:
            strncpy(info->my_prio, "LEVEL_3", INFO_STR_LENGTH);
            break;
        case LEVEL_4:
            strncpy(info->my_prio, "LEVEL_4", INFO_STR_LENGTH);
            break;
        case LEVEL_IDLE:
            strncpy(info->my_prio, "LEVEL_IDLE", INFO_STR_LENGTH);
            break;
        default:
            strncpy(info->my_prio, "UNKNOWN", INFO_STR_LENGTH);
        }

        info->my_prio[15] = '\0';

        info->child_amount = p->child_amount;
        for (int j = 0; j < MAX_PCS; j++)
            info->children[j] = p->childs[j];

        // File descriptors
        int fdCount = 0;
        for (int j = 0; j < MAX_FD; j++)
        {
            info->file_descriptors[j] = p->file_descriptors[j];
            if (p->file_descriptors[j] != 0)
                fdCount++;
        }
        info->fds_count = fdCount;
    }

    return list;
}

int get_pid()
{

    int pid = -1;
    for (int i = 0; i < MAX_PCS; i++)
    {
        if (process_table[i] != NULL && process_table[i]->state == RUNNING)
        {
            pid = process_table[i]->PID;
            break;
        }
    }

    return pid;
}

int is_pid_valid(int pid)
{
    if (pid < 0 || pid >= MAX_PCS)
        return 0;
    if (process_table[pid] == NULL)
        return 0;
    return 1;
}

int wait(uint64_t target_pid, uint64_t my_pid)
{

    if (!is_pid_valid(target_pid))
        return -1;

    if (!is_pid_valid(my_pid))
        return -2;

    PCB *target = process_table[target_pid]; // a quien tenemos q esperar

    // si el target termino primero, lo terminamos
    if (target->state == ZOMBIE)
    {
        return 0;
    }

    target->dad_blocked = 1;
    block_process(my_pid);
    return 0;
}

int get_shell_pid()
{
    return SHELL_PID;
}

int get_idle_pid()
{
    return IDLE_PID;
}