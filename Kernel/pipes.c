#include "pipes.h"
#include "sem.h"
#include "memory_manager.h" 
#include "lib.h"
#include <stddef.h>

#define MAX_PIPES            16 
#define PIPE_BUFFER_SIZE     1024    // Tamaño del buffer
#define MAX_PIPE_NAME_LENGTH 64

typedef struct {
    // Informacion del pipe
    int is_named;
    char name[MAX_PIPE_NAME_LENGTH];
   
    int ref_count;

    //Datos del pipe
    char buffer[PIPE_BUFFER_SIZE];
    int read_index;                 //Indice de lectura
    int write_index;                //Indice de escritura
    //int data_size; //Por ahora no lo usamos
    
    //Semaforos del pipe
    char sem_pipe_lock_name[MAX_NAME_LENGTH];
    char sem_items_available_name[MAX_NAME_LENGTH];
    char sem_empty_space_available_name[MAX_NAME_LENGTH];
    
    //Para almacenamiento interno
    int is_in_use;
} Pipe;

// Tabla Global de Pipes 
Pipe global_pipe_table[MAX_PIPES];

static int create_pipe_object(int pipe_id, const char *name, int is_named);
static void build_sem_names(const char *base_name, Pipe *pipe);
static void itoa(int n, char s[], int base);
static void reverse(char s[]);
static int strlen_custom(const char *s);
static void strcopy(char *dest, const char *src);
static void strncopy(char *dest, const char *src, unsigned int n);
static char *strcat(char *dest, const char *src);
static char *strncat(char *dest, const char *src, unsigned int n);


void pipe_init() {
    // Inicializa el semaforo para la tabla global
    sem_open_init("GLOBAL_PIPE_TABLE_LOCK", 1);

    for (int i = 0; i < MAX_PIPES; i++) {
        global_pipe_table[i].is_in_use = 0;
    }
}

int pipe_create_anonymous(int pipe_ids[2]) {
    // Se bloquea la tabla global de pipes
    sem_wait("GLOBAL_PIPE_TABLE_LOCK");

    int free_slot = -1;
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!global_pipe_table[i].is_in_use) {
            free_slot = i;
            global_pipe_table[i].is_in_use = 1; 
            break;
        }
    }

    if (free_slot == -1) {
        sem_post("GLOBAL_PIPE_TABLE_LOCK");
        return -1;
    }
    
    // Generacion de nombre para el pipe anonimo
    char anon_name[MAX_PIPE_NAME_LENGTH];
    strncopy(anon_name, "anon_", 5);
    char slot_str[10];
    itoa(free_slot, slot_str, 10);
    strcat(anon_name, slot_str);

    int pipe_id = create_pipe_object(free_slot, anon_name, 0);
    
    if (pipe_id < 0) {
        global_pipe_table[free_slot].is_in_use = 0;
        sem_post("GLOBAL_PIPE_TABLE_LOCK");
        return -1;
    }

    global_pipe_table[pipe_id].ref_count = 2;
    sem_post("GLOBAL_PIPE_TABLE_LOCK");

    //Devuelve los dos handles
    pipe_ids[0] = pipe_id; // Handle para pipe_read
    pipe_ids[1] = pipe_id; // Handle para pipe_write
    
    return 0;
}

int pipe_create_named(const char* name) {
    if (name == NULL)
        return -1;

    sem_wait("GLOBAL_PIPE_TABLE_LOCK");

    // Verificar si existe un pipe con ese nombre
    // si existe, incrementar ref_count y devolver su ID
    for (int i = 0; i < MAX_PIPES; i++) {
        if (global_pipe_table[i].is_in_use &&
            global_pipe_table[i].is_named &&
            strcmp(global_pipe_table[i].name, name) == 0) 
        {
            global_pipe_table[i].ref_count++;
            sem_post("GLOBAL_PIPE_TABLE_LOCK");
            return i;
        }
    }

    int free_slot = -1;
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!global_pipe_table[i].is_in_use) {
            free_slot = i;
            global_pipe_table[i].is_in_use = 1;
            break;
        }
    }

    if (free_slot == -1) {
        sem_post("GLOBAL_PIPE_TABLE_LOCK");
        return -1;
    }

    int pipe_id = create_pipe_object(free_slot, name, 1);

    //Si algo fallo al crear el pipe, liberar el slot
    if (pipe_id < 0) {
        global_pipe_table[free_slot].is_in_use = 0;
        sem_post("GLOBAL_PIPE_TABLE_LOCK");
        return -1;
    }

    global_pipe_table[pipe_id].ref_count = 1;
    sem_post("GLOBAL_PIPE_TABLE_LOCK");

    return pipe_id;
}


int pipe_close(int pipe_id) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES) {
        return -1;
    }

    sem_wait("GLOBAL_PIPE_TABLE_LOCK");

    if (!global_pipe_table[pipe_id].is_in_use) {
        sem_post("GLOBAL_PIPE_TABLE_LOCK");
        return -1;
    }

    Pipe* pipe = &global_pipe_table[pipe_id];

    // Disminuir el contador de referencias y si no hay mas referencias liberar recursos 
    pipe->ref_count--;
    if (pipe->ref_count == 0) {
        sem_close(pipe->sem_pipe_lock_name);
        sem_close(pipe->sem_items_available_name);
        sem_close(pipe->sem_empty_space_available_name);
        pipe->is_in_use = 0;
    } 

    sem_post("GLOBAL_PIPE_TABLE_LOCK");

    return 0;
}

int pipe_write(int pipe_id, const char* buffer, int count) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES || buffer == NULL || count <= 0 || !global_pipe_table[pipe_id].is_in_use) {
        return -1;
    }

    Pipe* pipe = &global_pipe_table[pipe_id];

    int bytes_written = 0;

    // Escribimos byte por byte
    for (int i = 0; i < count; i++) {
        // Esperamos por espacio disponible en el pipe
        sem_wait(pipe->sem_empty_space_available_name);

        // Entramos a la region critica
        sem_wait(pipe->sem_pipe_lock_name);

        // Escribir el byte
        pipe->buffer[pipe->write_index] = buffer[i];
        pipe->write_index = (pipe->write_index + 1) % PIPE_BUFFER_SIZE;

        // Salimos de la region critica
        sem_post(pipe->sem_pipe_lock_name);

        // Avisamos que hay cosas para leer
        sem_post(pipe->sem_items_available_name);

        bytes_written++;
    }

    return bytes_written;
}

int pipe_read(int pipe_id, char* buffer, int count) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES || buffer == NULL || count <= 0 || !global_pipe_table[pipe_id].is_in_use) {
        return -1;
    }

    Pipe* pipe = &global_pipe_table[pipe_id];

    int bytes_read = 0;

    // Lectura byte por byte
    for (int i = 0; i < count; i++) {
        // Se espera a que haya datos disponibles
        sem_wait(pipe->sem_items_available_name);

        // Entramos a la region critica
        sem_wait(pipe->sem_pipe_lock_name);

        // Leer un byte del buffer del pipe
        buffer[i] = pipe->buffer[pipe->read_index];
        pipe->read_index = (pipe->read_index + 1) % PIPE_BUFFER_SIZE;

        // Salimos de la region critica
        sem_post(pipe->sem_pipe_lock_name);

        //Avisamos que hay espacio
        sem_post(pipe->sem_empty_space_available_name);

        bytes_read++;
    }

    return bytes_read;
}

//Funciones auxiliares

static void build_sem_names(const char* base_name, Pipe* pipe) {
    // Lock
    strncpy(pipe->sem_pipe_lock_name, "p_lock_", MAX_NAME_LENGTH - 1);
    strncat(pipe->sem_pipe_lock_name, base_name, MAX_NAME_LENGTH - strlen_custom(pipe->sem_pipe_lock_name) - 1);
    
    // Items
    strncpy(pipe->sem_items_available_name, "p_items_", MAX_NAME_LENGTH - 1);
    strncat(pipe->sem_items_available_name, base_name, MAX_NAME_LENGTH - strlen_custom(pipe->sem_items_available_name) - 1);

    // Space
    strncpy(pipe->sem_empty_space_available_name, "p_space_", MAX_NAME_LENGTH - 1);
    strncat(pipe->sem_empty_space_available_name, base_name, MAX_NAME_LENGTH - strlen_custom(pipe->sem_empty_space_available_name) - 1);
}

static int create_pipe_object(int pipe_id, const char *name, int is_named) {
    Pipe *pipe = &global_pipe_table[pipe_id];

    strcopy(pipe->name, name);
    pipe->is_named = is_named;
    pipe->read_index = 0;
    pipe->write_index = 0;
    //pipe->data_size = 0;

    build_sem_names(name, pipe);

    if (sem_open_init(pipe->sem_pipe_lock_name, 1) < 0)
        return -1;

    if (sem_open_init(pipe->sem_items_available_name, 0) < 0) {
        sem_close(pipe->sem_pipe_lock_name);
        return -1;
    }

    if (sem_open_init(pipe->sem_empty_space_available_name, PIPE_BUFFER_SIZE) < 0) {
        sem_close(pipe->sem_pipe_lock_name);
        sem_close(pipe->sem_items_available_name);
        return -1;
    }

    return pipe_id;
}

static int strlen_custom(const char *s) {
    int n = 0;
    while (s[n] != '\0')
        n++;
    return n;
}

static void strcopy(char *dest, const char *src) {
    while ((*dest++ = *src++) != '\0');
}

static void strncopy(char *dest, const char *src, unsigned int n) {
    unsigned int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

static char *strcat(char *dest, const char *src) {
    char *ret = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++));
    return ret;
}

static char *strncat(char *dest, const char *src, unsigned int n) {
    char *ret = dest;
    while (*dest)
        dest++;
    while (n-- && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}

static void reverse(char s[]) {
    int i, j;
    char c;
    for (i = 0, j = strlen_custom(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

static void itoa(int n, char s[], int base) {
    int i = 0, sign = n;

    if (n < 0)
        n = -n;

    do {
        int digit = n % base;
        s[i++] = (digit < 10) ? digit + '0' : digit - 10 + 'A';
    } while ((n /= base) > 0);

    if (sign < 0)
        s[i++] = '-';

    s[i] = '\0';
    reverse(s);
}