#include "include/userlib.h"
#include "include/shell.h"
#include "include/userlibasm.h"
#include <stdint.h>

#define COMMANDS 14
#define TESTS 4
#define HELP 10
#define VERT_SIZE 32
#define LINE_SIZE 63
#define BUFFER_SIZE 128
#define MAX_ARGS 10

// Esto es un "string" manual para poder imprimir el caracter 128 de nuestro font de kernel usando lsa funciones estandar
char PROMPT_START[] = {127, 0};
#define ERROR_PROMPT "Unknown command: "

// Buffers
char screen_buffer[VERT_SIZE][LINE_SIZE];
char command_buffer[BUFFER_SIZE];
static char* commands[COMMANDS] = {"exit", "clear","sleep", "infoSleep", "help", "registers", "test-div", "test-invalid", 
    "test-mm", "test-prio", "test-pcs", "test-sync", "mem", "Tests"};
static char* tests[TESTS] = {"test-mm", "test-prio", "test-pcs", "test-sync"};
static char* help[HELP] = {"exit", "clear","sleep", "infoSleep", "help", "registers", "test-div", "test-invalid", "mem", "Tests"};
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
        
        write_out(PROMPT_START);
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

    // a partir de aca si esta lleno el buffer nos vamos
    if (command_cursor == BUFFER_SIZE - 1) 
        return;

    else {
        command_buffer[command_cursor++] = key;
        write_out(char_buffer);
    }
}

static void remove_extra_spaces(char *str);
static int split_arguments(char *buffer, char *argv[], int max_args);
static int remove_first_argument(char *argv[], int argc);

void process_command(char* buffer){
    char *argv[MAX_ARGS];

    if (buffer[0] == '\0'){
        return;
    }

    remove_extra_spaces(buffer);
    if (buffer[0] == '\0') {
        write_out(PROMPT_START);
        return;
    }

    int argc = split_arguments(buffer, argv, MAX_ARGS);

   for(int i = 0; i < COMMANDS; i++){
        if (!strcmp(buffer, commands[i])){
            switch (i) {
                case 0:
                    write_out("Nos vemos, esperamos que la hayas pasado bien! \n");
                    bye[0]= "0";
                    bye[1]= NULL;
                    exit_pcs(EXIT);
                    break;
                case 1:
                    clearScreen(); 
                    cursor_y = 0;
                    cursor_x = 0;
                    limit_index = VERT_SIZE/font_size - 1;
                    break;
                case 2:
                    write_out("Vamos a esperar 4 segundos... ");
                    sleep(4, 0);
                    write_out("Listo!\n");
                    break;
                case 3:
                    write_out("El comando sleep efectuara una espera de 4 segundos para demostrar el funcionamiento de la syscall. Los comandos milisleep y nanosleep haran algo equivalente pero con sus respectivas unidades\n");
                    break;
                case 4:
                    write_out("Los comandos existentes son:\n");
                    for(int i=0; i<HELP; i++){
                        write_out(help[i]);
                        write_out("\n");
                    }
                    break;
                case 5:
                    if(getRegs(regs)==0){
                        write_out("Antes de pedir los registros debe apretar la tecla alt izquierda para que los mismos se guarden\n");
                    }

                    else{
                        for(int i=0; i<cantRegs; i++){
                            if (i != cantRegs - 1)
                                write_out("-");
                            write_out(regsNames[i]);
                            uintToBase(regs[i], aux, 10);
                            write_out(aux);
                            write_out("\n");
                        }
                    }
                    break;
         
                case 6:
                    write_out("Vamos a testear dividir 1 por 0 en:\n");
                    write_out("3...\n");
                    sleep(1, 0);
                    write_out("2...\n");
                    sleep(1, 0);
                    write_out("1...\n");
                    sleep(1, 0);
                    int a = 1;
                    int b = 0;
                    if (a/b ==1)
                        write_out("You really shouldnt be here chief... medio que rompiste la matematica\n");
                    break;

                case 7:
                    write_out("Vamos a tratar de desafiar al runtime de asm en:\n");
                    write_out("3...\n");
                    sleep(1, 0);
                    write_out("2...\n");
                    sleep(1, 0);
                    write_out("1...\n");
                    sleep(1, 0);
                    _opError();    
                    break;

                case 8: 
                    argc = remove_first_argument(argv, argc);
                    test_mm(argc, argv);
                    break;

                case 9:
                    argc = remove_first_argument(argv, argc);
                    test_prio(argc, argv);
                    break;

                case 10:
                    argc = remove_first_argument(argv, argc);
                    test_pcs(argc, argv);
                    break;

                case 11:
                    argc = remove_first_argument(argv, argc);
                    test_sync(argc, argv);
                    break;

                case 12:
                    status_count(argc, argv);
                    break;
                    
                case 13:
                    write_out("Los Tests existentes son:\n");
                    for(int i=0; i<TESTS; i++){
                        write_out(tests[i]);
                        write_out("\n");
                    }
                    break;
            }   
            return;
        }
    }

    if (strlen(buffer) == BUFFER_SIZE){
        write_out("Buenas... una poesia?\n");
        return;
    }

    // En caso de no encontrar hacemos esto
    cursor_x = 0;
    write_out(ERROR_PROMPT);
    write_out(buffer);
    write_out("\n");
    return;
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

// Llenamos argv[]
static int split_arguments(char *buffer, char *argv[], int max_args) {
    int argc = 0;
    int i = 0;
    char *start = 0;
    int in_token = 0;

    while (buffer[i]) {
        char c = buffer[i];
        if (c != ' ') {
            if (!in_token) {
                start = &buffer[i];
                in_token = 1;
            }
        } else {
            if (in_token) {
                buffer[i] = '\0';
                if (argc < max_args - 1)
                    argv[argc++] = start;
                in_token = 0;
            }
        }
        i++;
    }

    if (in_token && argc < max_args - 1)
        argv[argc++] = start;

    argv[argc] = 0;
    return argc;
}

//La usamos para sacar el comando del argv q le pasamos a los test :)
int remove_first_argument(char *argv[], int argc) {
    int i = 0;

    while (argv[i] != 0) {
        argv[i] = argv[i + 1];
        i++;
    }

    return argc-1;
}