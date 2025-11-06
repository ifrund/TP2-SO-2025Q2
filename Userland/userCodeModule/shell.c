#include "include/userlib_so.h"
#include "include/shell.h"
#include "include/userlibasm.h"
#include "include/file_descriptors.h"
#include <stdint.h>

#define COMMANDS 23
#define TESTS 4
#define SOCOMS 1
#define VERT_SIZE 32
#define LINE_SIZE 63
#define BUFFER_SIZE 128
#define MAX_ARGS 10

// Funciones dummy de userlib.c que find_command_rip necesita
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
extern void msg_dummy(int argc, char **argv);

// Esto es un "string" manual para poder imprimir el caracter 128 de nuestro font de kernel usando lsa funciones estandar
#define ERROR_PROMPT "Unknown command: "
char PROMPT_START[] = {127, 0};
int kill_from_shell = 0, foreground = 1;
int current_foreground_pid;
// Buffers
char screen_buffer[VERT_SIZE][LINE_SIZE];
char command_buffer[BUFFER_SIZE];
static char* commands[COMMANDS] = {"exit", "clear","sleep", "infoSleep", "help", "registers", "test-mm", "test-prio", 
    "test-pcs", "test-sync", "mem", "Tests", "kill", "ps", "nice", "block", "unblock", "loop", "wc", "cat", "filter", "mvar", "msg"};
static char* tests[TESTS] = {"test-mm", "test-prio", "test-pcs", "test-sync"};
static char* help[COMMANDS-TESTS] = {"exit", "clear", "sleep", "infoSleep", "help", "registers", "mem", "Tests",
    "kill", "ps", "nice", "block", "unblock", "loop", "wc", "cat", "filter", "mvar", "msg"};
static char* info[COMMANDS-TESTS] = {"-", "-", "-por ahora", "-", "-", "-", "-", "-", "(el pid a matar)", "-", "(pid a afectar) (nueva prioridad)", 
    "(pid a bloquear)", "(pid a desbloquear)", "(segundos entre aparaciones del loop)", "-", "-", "-", "-", "<-- eliminar"};

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
    }

    clearScreen();

    return 0;
}


void process_key(char key){
    if (key == '\n'){

        command_buffer[command_cursor] = '\0';

        write_out("\n");
        process_command(command_buffer);

        //Limpieza profunda del buffer
        for (int i = 0; i <= BUFFER_SIZE; i++) {
            command_buffer[i] = 0;
        }

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
        write_out("Esto es ctrl+d, tdv no esta desarrollado.\n"); //TODO
        write_out(PROMPT_START);
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
    char* command_name;
    char* args_string = NULL; // String que contiene solo los argumentos

    // Se busca background o pipe
    char* bg_pos = strchr(buffer, '&');
    if (bg_pos != NULL) {
        foreground = 0;
        *bg_pos = '\0'; // Elimina el '&'
    }

    char* pipe_pos = strchr(buffer, '|');
    if (pipe_pos != NULL) {
        // Hay pipe
        *pipe_pos = '\0';
        char* cmd_A = buffer;
        char* cmd_B = pipe_pos + 1;
        handle_pipe_command(cmd_A, cmd_B, foreground);

    } else {
        // Hay un unico comando sin pipe
        remove_extra_spaces(buffer);

        command_name = buffer;
        char* first_space = strchr(buffer, ' ');

        if (first_space != NULL) {
            // Si hay un espacio, separamos el comando de los argumentos
            *first_space = '\0';
            args_string = first_space + 1;
            while (*args_string == ' ') args_string++;
        }
        
        argc = parse_arguments(args_string, argv);
        
        //Checkear si esta vacio y si no, buscar el RIP del comando
        if (command_name[0] == '\0') return; 
        void* rip = find_command_rip(command_name);
        
        if (rip == NULL) {
            // Caso comandos built in
            if (strcmp(command_name, "exit") == 0) {
                exit_shell();
            
            } else if (strcmp(command_name, "clear") == 0) {
                clearScreen(); 
                cursor_y = 0;
                cursor_x = 0;
                limit_index = VERT_SIZE/font_size - 1;
            
            } else if (strcmp(command_name, "sleep") == 0) {
                 write_out("Vamos a esperar 4 segundos... ");
                 sleep(4, 0);
                 write_out("Listo!\n");
            
            } else if (strcmp(command_name, "infoSleep") == 0) {
                 write_out("El comando sleep efectuara una espera de 4 segundos...\n");
            
            } else if (strcmp(command_name, "help") == 0) {
                write_out("Los comandos existentes son:\n");
                for(int i=0; i<(COMMANDS-TESTS); i++){
                    write_out(help[i]);
                    write_out(" ");
                    write_out(info[i]);
                    write_out("\n");
                }
            
            } else if (strcmp(command_name, "registers") == 0) {
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
            } else if (strcmp(command_name, "Tests") == 0){
                write_out("Los test existentes son:\n");
                for(int i=0; i<(TESTS); i++){
                    write_out(tests[i]);
                    write_out("\n");
                }
            } 
            else {

                if (strlen(buffer) == BUFFER_SIZE){
                    write_out("Buenas... una poesia?\n");
                    foreground = 1;
                    return;
                }

                cursor_x = 0;
                write_out(ERROR_PROMPT);
                write_out(command_name);
                write_out("\n");
            }
        } else {
            // Caso proceso normal
            //write_out("Iniciando proceso...\n");
            
            int pid = create_process(rip, command_name, argc, argv);

            if (foreground && pid > 0) {
                current_foreground_pid = pid;
                _update_foreground(current_foreground_pid);
                int myPid = _get_pid();
                _wait(pid, myPid, command_name);
                current_foreground_pid = shell_pid;
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
    line_size = LINE_SIZE/font_size;\
    //clearScreen(); TODO: VER SI SACARLO
    shell_pid = _shell_pid();
    idle_pid = _idle_pid();
    current_foreground_pid = shell_pid;
    _update_foreground(current_foreground_pid);
}

//Helpers

static void handle_pipe_command(char* cmd_A, char* cmd_B, int foreground) {
    char *argv_A[MAX_ARGS], *argv_B[MAX_ARGS];
    int argc_A, argc_B;
    char* command_A, *command_B;
    char* args_A = NULL, *args_B = NULL;
    char* first_space;

    remove_extra_spaces(cmd_A);
    remove_extra_spaces(cmd_B);
    
    // Se parsean ambos comandos, A y B
    command_A = cmd_A;
    first_space = strchr(cmd_A, ' ');
    if (first_space != NULL) {
        *first_space = '\0';
        args_A = first_space + 1;
        while (*args_A == ' ') args_A++;
    }
    argc_A = parse_arguments(args_A, argv_A); 
    
    command_B = cmd_B;
    first_space = strchr(cmd_B, ' ');
    if (first_space != NULL) {
        *first_space = '\0';
        args_B = first_space + 1;
        while (*args_B == ' ') args_B++;
    }
    argc_B = parse_arguments(args_B, argv_B);

    //Se buscan ambos RIPs
    void* rip_A = find_command_rip(command_A);
    void* rip_B = find_command_rip(command_B);

    if (rip_A == NULL || rip_B == NULL) {
        write_out("Error: comando invalido en el pipe.\n");
        return;
    }

    // Manejo de pipes
    int pipe_ids[2];
    if (_pipe_create_anonymous(pipe_ids) == -1) {
        write_out("Error: no se pudo crear el pipe.\n");
        return;
    }

    printDec(pipe_ids[0]);
    write_out(" ");
    printDec(pipe_ids[1]);
    uint64_t fds_A[2] = {STDIN, pipe_ids[1]};
    uint64_t fds_B[2] = {pipe_ids[0], STDOUT};

    int pid_A = create_process_piped(rip_A, command_A, argc_A, argv_A, fds_A);
    int pid_B = create_process_piped(rip_B, command_B, argc_B, argv_B, fds_B);

    // Esperar si es foreground
    if (foreground) {
        int myPid = _get_pid();
        _wait(pid_A, myPid, command_A);
        _wait(pid_B, myPid, command_B);
    }

    // Cerrar ambos extremos del pipe en la shell
    _pipe_close(pipe_ids[0]);
    _pipe_close(pipe_ids[1]);
    
}

static int parse_arguments(char* buffer, char** argv) {
    int argc = 0;

    if (buffer == NULL || *buffer == '\0') {
        argv[0] = NULL;
        return 0;
    }

    argv[argc++] = buffer; 

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

static void* find_command_rip(char* name) {
    // Array de punteros a funciones (o sea los RIPs)
    // MISMO ORDEN QUE COMMANDS
    static void* command_rips[COMMANDS] = {
        NULL,                   // "exit"
        NULL,                   // "clear"
        NULL,                   // "sleep"
        NULL,                   // "infoSleep"
        NULL,                   // "help"
        NULL,                   // "registers"
        &test_mm_dummy,         // "test-mm"
        &test_prio_new,         // "test-prio"
        &test_processes_dummy,  // "test-pcs"
        &test_sync_dummy,       // "test-sync"
        &status_count_dummy,    // "mem"
        NULL,                   // "Tests"
        &kill_dummy,            // "kill"
        &get_proc_list_dummy,   // "ps"
        &be_nice_dummy,         // "nice"
        &block_process_dummy,   // "block"
        &unblock_process_dummy, // "unblock"
        &loop_dummy,            // "loop"
        &wc_dummy,              // "wc"
        &cat_dummy,             // "cat"
        &filter_dummy,          // "filter"
        &mvar_dummy,             // "mvar"
        &msg_dummy               // "msg"
    };

    for (int i = 0; i < COMMANDS; i++) {
        if (strcmp(name, commands[i]) == 0) {
            return command_rips[i];
        }
    }
    return NULL;
}

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
    return NULL;
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