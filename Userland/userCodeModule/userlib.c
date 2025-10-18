#include "include/userlib.h"
#include "include/userlibasm.h"
#include "include/file_descriptors.h"
#include "include/shell.h"
#include <stdint.h>

static char buffer[64] = {'0'};
static char* char_buffer = " ";

//================================================================================================================================
// Writting
//================================================================================================================================
//================================================================================================================================
void printChar(char charToPrint){
    buffer[0] = charToPrint;
    print(buffer);
}

void print(char * string){
    _print(STDOUT, string, strlen(string));
}

void printCant(char* string, int cant){
    _print(STDOUT, string, cant);
}

void printError(char * string){
    _print(STDERROR, string, strlen(string));
}

void clearScreen(){
    _print(STDOUT, "\033[J", 3);
}

void flushBuffer(){
    _print(STDOUT, "\033[C", 3);
}

void change_font(int size){
    char* msg = "\033[nF";
    msg[2] = size + '0';
    print(msg);
}

int read(char* buffer, int length){
    return _read(STDIN, buffer, length);
}

int readRaw(char* buffer, int length){
    return _read(STDKEYS, buffer, length);
}

int readLast(char* buffer, int length){
    return _read(STDLAST, buffer, length);
}

void printBase(uint64_t value, uint32_t base){
    uintToBase(value, buffer, base);
    print(buffer);
}

void printDec(uint64_t value){
    printBase(value, 10);
}

void printHex(uint64_t value){
    printBase(value, 16);
}




//================================================================================================================================
// General use
//================================================================================================================================
//================================================================================================================================
uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	//Calculate characters for each digit
	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	}
	while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	//Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}

int strcmp(const char *str1, const char *str2){
    while (*str1 && (*str1 == *str2)){
        str1++;
        str2++;
    }

    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int strlen(char * string){
    int i=0;
    while(string[i++]!=0);
    return i;
}

void strcpy(char *destination, const char *source) {
    while (*source != '\0') {
        *destination++ = *source++;
    }
    *destination = '\0';
}


int mod(int val, int base){
    if (val < 0) return (val + base) % base;
    return val % base;
}

int is_digit(char c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

int char_to_int(const char* str) {
    int result = 0;

    if (str == NULL || *str == '\0') {
        write_out("Parametro invalido: la cadena esta vacia o es nula.\n");
        exit_pcs(ERROR);
    }

    while (*str != '\0') {
        if (!is_digit(*str)) {
            write_out("Parametro invalido: se esperaba un numero entero positivo.\n");
            exit_pcs(ERROR);
        }
        uint32_t digit = *str - '0';
        if (result > (UINT32_MAX - digit) / 10) {
            write_out("Parametro invalido: el numero es demasiado grande.\n");
            exit_pcs(ERROR);
        }
        result = result * 10 + digit;
        str++;
    }

    return result;
}

//================================================================================================================================
// Sleep
//================================================================================================================================

void sleep(uint32_t cant, uint32_t unidad){
	_sleep(cant, unidad);
}

void sleep_once(){
    _sleep(0, 1);
}

//================================================================================================================================
// Clock
//================================================================================================================================
void getClock(int *hrs, int *min, int *seg){
	_getClock(hrs, min, seg);
}

//================================================================================================================================
// Drawing
//================================================================================================================================
//================================================================================================================================
void getScreenData(uint16_t * screenHeight, uint16_t * screenWidth, uint8_t * fontSize, uint8_t * drawSize){
	_screenData(screenHeight,screenWidth,fontSize,drawSize);
}

int getFontSize(){
    // estos estan inicializados porq sino se rompe la funcion al querer escribir vacio
    uint16_t bufferHeight = 0;
    uint16_t bufferWeight = 0;
    uint8_t bufferDraw = 0;
    _screenData(&bufferHeight, &bufferWeight, (uint8_t*) char_buffer,&bufferDraw);
    return (int) char_buffer[0];
}


void draw(uint16_t * bitmap, uint32_t color, uint16_t height, uint64_t x, uint64_t y){
	_draw(bitmap, color, height, x, y);
}

void changeDrawSize(uint8_t newSize){
	_changeSize(newSize, 2);
}

//================================================================================================================================
// Registers
//================================================================================================================================

int getRegs(uint64_t regs[]){
    return _getRegs(regs);
}

//================================================================================================================================
// Beep!
//================================================================================================================================
void beep(uint32_t frequency, int duration){
    _beep(frequency, duration);
}


/*================================================================================================================================
SISTEMAS OPERATIVOS
================================================================================================================================*/


// Crear la memoria
void create_mm(){
    _create_mm();
}

// Ocupa espacio en la memoria
void *alloc(int size){
    return _alloc(size);
}

// Libera espacio de la memoria
void free(void* address){
    _free(address);
}

// Llena el array con los datos block_count, free_space, y used_space 
void status_count(int *status_out){
    _status_count(status_out);
}

int create_process(void * rip , const char *name, int argc, char *argv[]){
    return _create_process(rip, name, argc, argv);
}

void kill_dummy(int argc, char ** argv){

    if (argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        exit_pcs(ERROR);    
    }

    int toKill = char_to_int(argv[0]);
    if(toKill == 0){
        write_out("Para matar la shell tenes que usar Exit. \n");
        exit_pcs(EXIT);
    }
    if(toKill == _get_pid()){ //Estas matando al propio proceso kill que creaste
        write_out("Kill al kill ?? ... okay\n");
        exit_pcs(EXIT);
    }
    else{
        write_out("Chau chau al proceso de pid: ");
        write_out(argv[0]);
        write_out("\n");
    }

    _kill_process(toKill); 
    exit_pcs(EXIT);
}

//recibe el pid de a quien matar por argv
int kill_process(char * argv[], int argc){
    return create_process(&kill_dummy, "name", argc, argv);
}

//cada proceso cuando termina se debe "matar" a si mismo, osea dejar marcado con KILLED
void exit_pcs(int ret){
    
    int pid = _get_pid(); 

   /* TODO, esto debe imprimir en terminal, no en pantalla
    if(ret == ERROR){
        write_out("El proceso de pid ");
        printDec(pid);
        write_out(" cerro con error\n");
    }
    else if(ret == EXIT){
        write_out("Proceso de pid ");
        printDec(pid);
        write_out(" cerro bien :)\n");
    }
    */

    _kill_process(pid);
}

int block_process(uint64_t pid){
    return _block_process(pid);
}

int unblock_process(uint64_t pid){
   return _unblock_process(pid);
}

void get_proc_list(char ** procNames, uint64_t * pids, uint64_t * parentPids, char ** status, uint64_t * rsps){
    _get_proc_list(procNames, pids, parentPids, status, rsps);
}

//ésta no es proceso, es built-in porq sino devolveria su propio pid xd
int get_pid(){
    return _get_pid();
}

void yield(){
    _yield();
}

int be_nice(int pid){
    return _be_nice(pid);
}