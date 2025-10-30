#include "include/userlib.h"
#include "include/shell.h"
#include "include/userlibasm.h"
#include "include/file_descriptors.h"
#include <stdint.h>

#define COMMANDS 25
#define TESTS 4
#define SOCOMS 1
#define VERT_SIZE 32
#define LINE_SIZE 63
#define BUFFER_SIZE 128
#define MAX_ARGS 10

// --- PROTOTIPOS EXTERNOS ---
// (Funciones dummy de userlib.c que find_command_rip necesita)
extern void test_mm_dummy(int argc, char **argv);
extern uint64_t test_prio_new(uint64_t argc, char **argv);
extern void test_processes_dummy(int argc, char **argv);
extern void test_sync_dummy(int argc, char **argv);
extern void status_count_dummy(int argc, char **argv);
extern void kill_dummy(int argc, char **argv);
extern void get_proc_list_dummy(int argc, char **argv);
extern void be_nice_dummy(int argc, char **argv);
extern void block_process_dummy(int argc, char **argv);
extern void unblock_process_dummy(int argc, char **argv);
extern void loop_dummy(int argc, char **argv);
extern void wc_dummy(int argc, char **argv);
extern void cat_dummy(int argc, char **argv);
extern void filter_dummy(int argc, char **argv);
extern void mvar_dummy(int argc, char **argv);
// --- FIN DE PROTOTIPOS EXTERNOS ---

// Esto es un "string" manual para poder imprimir el caracter 128 de nuestro font de kernel usando lsa funciones estandar
#define ERROR_PROMPT "Unknown command: "
char PROMPT_START[] = {127, 0};
int kill_from_shell = 0, foreground = 1;
int current_foreground_pid = -1;
// Buffers
char screen_buffer[VERT_SIZE][LINE_SIZE];
char command_buffer[BUFFER_SIZE];
static char* commands[COMMANDS] = {"exit", "clear","sleep", "infoSleep", "help", "registers", "test-div", "test-invalid", 
    "test-mm", "test-prio", "test-pcs", "test-sync", "mem", "Tests", "kill", "ps", "nice", "help-SO", "block", "unblock",
    "loop", "wc", "cat", "filter", "mvar"};

//static char* tests[TESTS] = {"test-mm", "test-prio", "test-pcs", "test-sync"};

static char* help[COMMANDS-TESTS] = {"exit", "clear","sleep", "infoSleep", "help", "registers", "test-div", "test-invalid", 
    "mem", "Tests", "kill", "ps", "nice", "help-SO", "block", "unblock", "loop", "wc", "cat", "filter", "mvar"};
// 
// static char* SOcommands[SOCOMS]= {
    // "Aca tiene q ir como funciona cada comando de SO"
// };

//REFACTOR
char char_buffer[1];

// Cursors & flags
int command_cursor = 0;
int cursor_y;
int cursor_x;
int exit_command;

// Important values
uint8_t font_size;
int rows_to_show;
int limit_index = VERT_SIZE - 1;
int line_size = LINE_SIZE;

int hrs = 0;
int min = 0; 
int sec = 0;

char aux[128];
int cantRegs = 18;
uint64_t regs[18];
char* regsNames[18] = {"rax:", "rbx:", "rcx:", "rdx:", "rsi:", "rdi:", "rbp:", "rsp:", "r8:", "r9:",
                       "r10:", "r11:", "r12:", "r13:", "r14:", "r15:", "rip:", "rflags:"};
char* bye[MAX_ARGS];
void exit_shell();

//Helpers
static void remove_extra_spaces(char *str);
static char* strchr(const char* str, int c);
static int parse_arguments(char* buffer, char** argv);
static void* find_command_rip(char* name);
static void handle_pipe_command(char* cmd_A, char* cmd_B, int foreground);

int shell(){
    cursor_x = 0;
    cursor_y = 0;
    exit_command = 0;

    clearScreen();
    init_shell();
    write_out(PROMPT_START);

    while(!exit_command){
        if (read(char_buffer, 1) == 1){
            process_key(char_buffer[0]);
        }
        sleep_once();
    }

    clearScreen();

    return 0;
}


void process_key(char key){
    if (key == '\n'){

        command_buffer[command_cursor] = '\0';

        write_out("\n");
        process_command(command_buffer);

        command_cursor = 0;
        if(foreground){
            write_out(PROMPT_START);
        }

        return;
    }

    if (key == '\b'){
        if (!command_cursor)
            return;

        command_cursor--;
        cursor_x = mod(cursor_x - 1, line_size);

        // aca va printChar y no write_out porq es un caso especial
        printChar(key);
        return;
    }

    if (key == '\x04') { //Ctrl+D
        write_out("Esto es ctrl+d, tdv no esta desarrollado.\n");
        // if (command_cursor == 0) {
        //     write_out("\nExit shell\n");
        //     exit_shell();
        // } else {
            
        //     //TODO
        // }
    }

    if (key == '\x03') { // Ctrl+C
        write_out("\n");  // para que no se mezcle con la línea actual

        if (current_foreground_pid > 0) {
            char pid_str[12];
            char *argv_kill[2];

            int_to_str(current_foreground_pid, pid_str);
            argv_kill[0] = pid_str;
            argv_kill[1] = NULL;

            kill_from_shell = 1;           // para que kill_dummy sepa que viene de la shell
            kill_process(1, argv_kill);    // matamos el proceso foreground

            current_foreground_pid = -1;   // limpiamos
            write_out("Proceso foreground terminado.\n");
        } else {
            write_out("No hay proceso en foreground para matar.\n");
    }

    write_out(PROMPT_START); // Volvemos al prompt
    return;
}


    // a partir de aca si esta lleno el buffer nos vamos
    if (command_cursor == BUFFER_SIZE - 1) 
        return;

    else {
        command_buffer[command_cursor++] = key;
        write_out(char_buffer);
    }
}

void process_command(char* buffer){
    char *argv[MAX_ARGS];
    int argc = 0;
    foreground = 1;

    // 1. Buscar background
    char* bg_pos = strchr(buffer, '&');
    if (bg_pos != NULL) {
        foreground = 0;
        *bg_pos = '\0'; // Elimina el '&'
    }

    // 2. Buscar pipe
    char* pipe_pos = strchr(buffer, '|');
    if (pipe_pos != NULL) {
        // --- CASO PIPE: "cmd A | cmd B" ---
        *pipe_pos = '\0'; // Divide el buffer
        char* cmd_A = buffer;
        char* cmd_B = pipe_pos + 1;
        
        handle_pipe_command(cmd_A, cmd_B, foreground);

    } else {
        // --- CASO COMANDO ÚNICO ---
        remove_extra_spaces(buffer);
        argc = parse_arguments(buffer, argv); // Parsea el comando
        
        if (argc == 0) return; // Comando vacío
        
        void* rip = find_command_rip(argv[0]); // Busca el RIP del comando
        
        if (rip == NULL) {
            // --- MANEJAR COMANDOS BUILT-IN ---
            if (strcmp(argv[0], "exit") == 0) {
                exit_shell();
            } else if (strcmp(argv[0], "clear") == 0) {
                clearScreen(); 
                cursor_y = 0;
                cursor_x = 0;
                limit_index = VERT_SIZE/font_size - 1;
            } else if (strcmp(argv[0], "sleep") == 0) {
                 write_out("Vamos a esperar 4 segundos... ");
                 sleep(4, 0);
                 write_out("Listo!\n");
            } else if (strcmp(argv[0], "infoSleep") == 0) {
                 write_out("El comando sleep efectuara una espera de 4 segundos...\n");
            } else if (strcmp(argv[0], "help") == 0) {
                write_out("Los comandos existentes son:\n");
                for(int i=0; i<(COMMANDS-TESTS); i++){
                    write_out(help[i]);
                    write_out("\n");
                }
            } else if (strcmp(argv[0], "registers") == 0) {
                if(getRegs(regs)==0){
                    write_out("Antes de pedir los registros debe apretar la tecla alt izquierda...\n");
                } else {
                    for(int i=0; i<cantRegs; i++){
                        if (i != cantRegs - 1) write_out("-");
                        write_out(regsNames[i]);
                        uintToBase(regs[i], aux, 10);
                        write_out(aux);
                        write_out("\n");
                    }
                }
            }
            // ... (Puedes añadir el resto de tus built-ins aquí) ...
            else {
                cursor_x = 0;
                write_out(ERROR_PROMPT);
                write_out(argv[0]);
                write_out("\n");
            }
        } else {
            // --- ES UN PROCESO, CREARLO ---
            // (La función create_process ahora usa STDIN/STDOUT por defecto)
            int pid = create_process(rip, argv[0], argc, argv);
            
            if (foreground && pid > 0) {
                current_foreground_pid = pid; 
                char pid_str[16];
                int_to_str(pid, pid_str);
                char *wait_argv[2];
                wait_argv[0] = pid_str;
                wait_argv[1] = NULL;
                wait(1, wait_argv);
                current_foreground_pid = -1; 
            }
        }
    }
}

void shift(){
    clearScreen();

    for (int i = 1; i < rows_to_show; i++){
        
        int line_number = mod(i + (limit_index - rows_to_show + 1), VERT_SIZE);
        print(screen_buffer[line_number]);
        if (i != rows_to_show - 1)
            print("\n");
    }
}

int check_shift(){
    int shifted = 0;
    if (cursor_y == limit_index){
        shift();
        shifted = 1;
        limit_index = (limit_index + 1) % VERT_SIZE;
    }

    cursor_y = (cursor_y + 1) % VERT_SIZE;
    return shifted;
}


void write_out(char* string){
    for (int c = 0; c < strlen(string)-1; c++){
        // el menos 1 es porq line_size es 1 mas que el maximo indice
        if (cursor_x == line_size - 1|| string[c] == '\n'){
            if (string[c] == '\n') 
                screen_buffer[cursor_y][cursor_x] = '\0'; // null terminate en caso de print
            else
                c--;    // se que parece raro pero esto esta para no escribir el \n en la tabla pero si los demas despues del shift
            check_shift();
            cursor_x = 0;
        }
        // ese else c-- esta para que pueda entrar a este else con el ultimo caracter de cada linea
        else {
            screen_buffer[cursor_y][cursor_x++] = string[c];
        }
    }

    print(string);
}


void init_shell(){
    font_size = getFontSize();
    rows_to_show = VERT_SIZE/font_size;
    line_size = LINE_SIZE/font_size;
}

//HELPERS
static void handle_pipe_command(char* cmd_A, char* cmd_B, int foreground) {
    char *argv_A[MAX_ARGS], *argv_B[MAX_ARGS];
    int argc_A, argc_B;

    remove_extra_spaces(cmd_A);
    remove_extra_spaces(cmd_B);
    argc_A = parse_arguments(cmd_A, argv_A);
    argc_B = parse_arguments(cmd_B, argv_B);

    if (argc_A == 0 || argc_B == 0) {
        write_out("Error: sintaxis de pipe invalida.\n");
        return;
    }

    void* rip_A = find_command_rip(argv_A[0]);
    void* rip_B = find_command_rip(argv_B[0]);

    if (rip_A == NULL || rip_B == NULL) {
        write_out("Error: comando invalido en el pipe.\n");
        return;
    }

    // 1. Crear el pipe anónimo (usando la syscall de userlibasm.h)
    int pipe_ids[2]; // pipe_ids[0] = READ, pipe_ids[1] = WRITE
    if (_pipe_create_anonymous(pipe_ids) == -1) {
        write_out("Error: no se pudo crear el pipe.\n");
        return;
    }
    
    // 2. Preparar los arrays de FDs
    uint64_t fds_A[2] = {STDIN, pipe_ids[1]}; // Proceso A: Escribe en el pipe
    uint64_t fds_B[2] = {pipe_ids[0], STDOUT}; // Proceso B: Lee del pipe

    // 3. Crear Proceso A (usando la syscall 'piped' de userlib.h)
    int pid_A = create_process_piped(rip_A, argv_A[0], argc_A, argv_A, fds_A);

    // 4. Crear Proceso B
    int pid_B = create_process_piped(rip_B, argv_B[0], argc_B, argv_B, fds_B);

    // 5. Cerrar ambos extremos del pipe en la SHELL (MUY IMPORTANTE)
    _pipe_close(pipe_ids[0]);
    _pipe_close(pipe_ids[1]);

    // 6. Esperar a que terminen (si es foreground)
    if (foreground) {
        char pid_str[16];
        char *wait_argv[2];
        wait_argv[1] = NULL;

        int_to_str(pid_A, pid_str);
        wait_argv[0] = pid_str;
        wait(1, wait_argv);

        int_to_str(pid_B, pid_str);
        wait_argv[0] = pid_str;
        wait(1, wait_argv);
    }
}

/**
 * @brief Parsea un string de comando (separado por espacios) en un argv.
 * Devuelve argc.
 */
static int parse_arguments(char* buffer, char** argv) {
    int argc = 0;
    if (buffer == NULL || *buffer == '\0') {
        return 0;
    }

    argv[argc++] = buffer; // El primer argumento es el comando mismo

    for (int i = 0; buffer[i] != '\0' && argc < MAX_ARGS - 1; i++) {
        if (buffer[i] == ' ') {
            buffer[i] = '\0';
            while (buffer[i+1] == ' ') {
                i++;
            }
            if (buffer[i+1] != '\0') { 
                argv[argc++] = &buffer[i + 1];
            }
        }
    }
    argv[argc] = NULL;
    return argc;
}

/**
 * @brief Busca en la lista de comandos y devuelve el puntero a la función
 * (RIP) del programa a ejecutar. Devuelve NULL si es un comando built-in o no existe.
 */
static void* find_command_rip(char* name) {
    // Array de punteros a funciones (RIPs). 
    // ¡DEBE ESTAR EN EL MISMO ORDEN QUE TU ARRAY 'commands'!
    static void* command_rips[COMMANDS] = {
        NULL,                   // "exit"
        NULL,                   // "clear"
        NULL,                   // "sleep"
        NULL,                   // "infoSleep"
        NULL,                   // "help"
        NULL,                   // "registers"
        NULL,                   // "test-div"
        NULL,                   // "test-invalid"
        &test_mm_dummy,         // "test-mm"
        &test_prio_new,         // "test-prio"
        &test_processes_dummy,  // "test-pcs"
        &test_sync_dummy,       // "test-sync"
        &status_count_dummy,    // "mem"
        NULL,                   // "Tests"
        &kill_dummy,            // "kill"
        &get_proc_list_dummy,   // "ps"
        &be_nice_dummy,         // "nice"
        NULL,                   // "help-SO"
        &block_process_dummy,   // "block"
        &unblock_process_dummy, // "unblock"
        &loop_dummy,            // "loop"
        &wc_dummy,              // "wc"
        &cat_dummy,             // "cat"
        &filter_dummy,          // "filter"
        &mvar_dummy             // "mvar"
    };

    for (int i = 0; i < COMMANDS; i++) {
        if (strcmp(name, commands[i]) == 0) {
            return command_rips[i];
        }
    }
    return NULL; // No encontrado
}

/**
 * @brief Implementación simple de strchr (busca un caracter en un string)
 */
static char* strchr(const char* str, int c) {
    while (*str != '\0') {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    if (c == '\0') {
        return (char*)str;
    }
    return NULL; // No encontrado
}

static void remove_extra_spaces(char *str) {
    int i = 0, j = 0;
    int in_space = 1; 

    while (str[i]) {
        char c = str[i++];
        if (c == ' ' || c == '\t' || c == '\n') {
            if (!in_space) {
                in_space = 1;
                str[j++] = ' ';
            }
        } else {
            in_space = 0;
            str[j++] = c;
        }
    }

    if (j > 0 && str[j - 1] == ' ')
        j--;

    str[j] = '\0';
}

void exit_shell(){
    write_out("Nos vemos, esperamos que la hayas pasado bien! \n");
    bye[0]= "0";
    bye[1]= NULL;
    exit_pcs(EXIT);
}

int read_input(char *buffer, int max_len) {
    int count = 0;
    int fake_c_c = command_cursor;
    char* fake_c_b = command_buffer;

    while (fake_c_c > 0 && count < max_len) {
        buffer[count++] = fake_c_b[0];
        for (int i = 1; i < fake_c_c; i++)
            fake_c_b[i-1] = fake_c_b[i];
        fake_c_c--;
    }

    return count;
}