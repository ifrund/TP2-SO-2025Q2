#include "include/userlib.h"

#define SHELL_PID 0
#define IDLE_PID 1

static char buffer[64] = {'0'};
static char* char_buffer = " ";

//================================================================================================================================
// Writting
//================================================================================================================================
//================================================================================================================================
void printChar(char charToPrint){
    buffer[0] = charToPrint;
    buffer[1] = '\0';
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
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    while (*str != '\0') {
        if (!is_digit(*str)) {
            write_out("Parametro invalido: se esperaba un numero entero positivo.\n");
            //write_out(PROMPT_START);
            exit_pcs(ERROR);
        }
        uint32_t digit = *str - '0';
        if (result > (UINT32_MAX - digit) / 10) {
            write_out("Parametro invalido: el numero es demasiado grande.\n");
            //write_out(PROMPT_START);
            exit_pcs(ERROR);
        }
        result = result * 10 + digit;
        str++;
    }

    return result;
}

void int_to_str(int value, char *str) {
    int i = 0, j;
    char temp[16];
    bool isNeg = false;

    if (value < 0) {
        isNeg = true;
        value = -value;
    }

    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (isNeg)
        temp[i++] = '-';

    for (j = 0; j < i; j++)
        str[j] = temp[i - j - 1];
    str[i] = '\0';
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
void alloc_dummy(int argc, char ** argv){

    if(argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);     
    }

    int size = char_to_int(argv[0]);

    _alloc(size);
    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int alloc(int argc, char ** argv){
    return create_process(&alloc_dummy, "alloc", argc, argv);
}

// Libera espacio de la memoria
void free_dummy(int argc, char ** argv){

    if(argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);     
    }

    _free((void*) argv[0]);
    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int free(int argc, char ** argv){
    return create_process(&free_dummy, "free", argc, argv);
}

// Llena el array con los datos block_count, free_space, y used_space 
void status_count_dummy(int argc, char ** argv){

    if (argc != 0) {
        write_out("No tenias que mandar argumentos para este comando.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    int status[3];
    _status_count(status);

    write_out("\n=== Estado del sistema de memoria ===\n");
    write_out("Bloques totales: ");
    printDec(status[0]);
    write_out("\nBloques usados: ");
    printDec(status[1]);
    write_out("\nBloques libres: ");
    printDec(status[2]);
    write_out("\n");

    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int status_count(int argc, char ** argv){
    return create_process(&status_count_dummy, "status count", argc, argv);
}

int create_process(void * rip , const char *name, int argc, char *argv[]){
    return _create_process(rip, name, argc, argv);
}

void kill_dummy(int argc, char ** argv){

    if (argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);    
    }

    int toKill = char_to_int(argv[0]);
    if(toKill == SHELL_PID){
        write_out("Para matar la shell tenes que usar Exit. \n");
        //write_out(PROMPT_START);
        exit_pcs(EXIT);
    }
    if(toKill == IDLE_PID){
        write_out("No te podemos permitir matar el idle ¯\\_(ツ)_/¯\n");
        //write_out(PROMPT_START);
        exit_pcs(EXIT);
    }
    if(toKill == _get_pid()){
        write_out("Kill al kill ?? ... okay\n");
        //write_out(PROMPT_START);
        exit_pcs(EXIT);
    }
    if(kill_from_shell == 1){
        kill_from_shell = 0;
        write_out("Chau chau al proceso de pid: ");
        write_out(argv[0]);
        write_out("\n");
    }

    int ret = _kill_process(toKill); 
    if(ret == ERROR){
        write_out("... O no\nEl pid ");
        printDec(toKill);
        write_out(" no es valido, asique no podemos matar a ningun proceso de ese pid... bobo.\n");
        //write_out(PROMPT_START);
        exit_pcs(EXIT);
    }

    //write_out("\n");
    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

//recibe el pid de a quien matar por argv
int kill_process(int argc, char ** argv){
    return create_process(&kill_dummy, "kill", argc, argv);
}

//cada proceso cuando termina se debe "matar" a si mismo, osea dejar marcado con KILLED
void exit_pcs(int ret){
    
    int pid = _get_pid(); 

    if(ret == ERROR){
        write_out("El proceso de pid ");
        printDec(pid);
        write_out(" cerro con error\n");
        write_out(PROMPT_START);
        exit_pcs(EXIT);
    }
    /* TODO estos write_out tienen q ir a la terminal, no a pantalla
    else if(ret == EXIT){
        write_out("Proceso de pid ");
        printDec(pid);
        write_out(" cerro bien :)\n");
    }
    */

    int ret_del_kill = _kill_process(pid);
    if(ret_del_kill == ERROR){
        write_out("Hubo un error en el exit ya que el pid ");
        if(pid==-1){
            write_out("-1");
        }
        else{
            printDec(pid);
        }
        write_out(" no es valido... big problem, no deberias llegar a aca nunca\n");
        //write_out(PROMPT_START);
        exit_pcs(EXIT);
    }
}

void block_process_dummy(int argc, char ** argv){

    if (argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);    
    }

    int pid = char_to_int(argv[0]);
    if(pid == 0){
        write_out("Por ahora la shell no es bloqueante, asiq... adios\n");
    }

    if(pid == get_pid()){
        write_out("Que haces loco, nos vas a meter en problemas raja de aca.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR); 
    }

    int ret = _block_process(pid);

    if(ret == ERROR){
        write_out("Este pid no es valido\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){
        write_out("Este pid ya estaba bloqueado\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR); 
    }

    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int block_process(int argc, char ** argv){
    return create_process(&block_process_dummy, "block", argc, argv);
}

void unblock_process_dummy(int argc, char ** argv){
   if (argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);    
    }

    int pid = char_to_int(argv[0]);

    if(pid == IDLE_PID){
        write_out("Que estas haciendo?? esto no sirve de nada, va a volver estar blocked cuando hagas ps.\n");
    }
    int ret = _unblock_process(pid);

    if(ret == ERROR){ 
        write_out("Este pid no es valido\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){ 
        write_out("Este proceso no esta bloqueado, asique no lo podemos desbloquear\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int unblock_process(int argc, char ** argv){
    return create_process(&unblock_process_dummy, "unblock", argc, argv);
}

void get_proc_list_dummy(){

    ProcessInfo* list = _get_proc_list();
    if (list == NULL) {
        write_out("Error, no se pudo obtener la lista de procesos.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
        return;
    }

    //encabezado
    write_out("\n=== Lista de procesos ===\n");
    write_out("PID\tNombre\tEstado\tPPID\tRSP\tPrio\tChilds\tFD\n");
    write_out("-------------------------------------------------------------\n");

    //iterar sobre la lista
    for (int i = 0; i < MAX_PCS; i++) {
        ProcessInfo* p = &list[i];
        if (p->pid == -1)  // Slot vacío
            continue;

        printDec(p->pid);
        write_out("\t");
        write_out(p->name);
        write_out("\t");
        write_out(p->state);
        write_out("\t");
        if (p->parentPid == (uint64_t)-1) write_out("-1");
        else printDec(p->parentPid);
        write_out("\t0x");
        printHex((uint64_t)p->rsp); 
        write_out("\t");
        write_out(p->my_prio);
        write_out("\t");
        printDec(p->childrenAmount);
        write_out("\t");
        printDec(p->fileDescriptorCount);
        write_out("\n");
    }

    write_out("-------------------------------------------------------------\n");

    _free(list);
    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int get_proc_list(int argc, char ** argv){
    return create_process(&get_proc_list_dummy, "ps", argc, argv);
}

//ésta no es proceso, es built-in porq sino devolveria su propio pid xd
int get_pid(){
    return _get_pid();
}

void yield_dummy(int argc, char ** argv){
    _yield();
    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int yield(int argc, char ** argv){
    return create_process(&yield_dummy, "yield", argc, argv);
}

void be_nice_dummy(int argc, char ** argv){

    if (argc != 2){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 2 argumentos.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);    
    }

    int pid = char_to_int(argv[0]);
    int new_prio = char_to_int(argv[1]);

    int ret = _be_nice(pid, new_prio);

    if(ret == ERROR){ 
        write_out("Este pid no es valido\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){
        write_out("Epa, intentaste cambiar la prioridad del idle, nonono\n");
        //write_out(PROMPT_START);
        exit_pcs(EXIT);
    }
    if(ret == -3){
        write_out("Mandaste una prioridad inexistente, porfavor acordate que las prioridades son de 0 a 4, siendo 0 la mayor\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int be_nice(int argc, char ** argv){
    return create_process(&be_nice_dummy, "nice", argc, argv);
}

int test_mm(int argc, char ** argv){
    return create_process(&test_mm_dummy, "test mm", argc, argv);
}

int test_prio(int argc, char ** argv){
    return create_process(&test_prio_new, "test prio", argc, argv);
}

int test_sync(int argc, char ** argv){
    return create_process(&test_sync_dummy, "test sync", argc, argv);
}

int test_pcs(int argc, char ** argv){
    return create_process(&test_processes_dummy, "test processes", argc, argv);
}


void loop_dummy(int argc, char ** argv){
  
    int pid = get_pid(); //agarro mi priopio pid
    if(argc!=1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }
    
    int cloop=0;
    int tiempo = char_to_int(argv[0]);
    while (1){
        write_out("Hola soy el loop, y mi pid es: ");
        printDec(pid);
        write_out(".\t Esta es mi vuelta ");
        printDec(cloop);
        cloop++;
        write_out(".\n");
        //write_out(PROMPT_START);
        sleep(tiempo, 0);
    }
}

int loop(int argc, char ** argv){
    return create_process(&loop_dummy,"loop", argc, argv);
}

void wait_dummy(int argc, char ** argv){
    
    if(argc!=1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    int pid = char_to_int(argv[0]);
    int ret = _wait(pid);

    if(ret == ERROR){
        write_out("Este pid no es valido\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){
        write_out("Como llegaste a aca?? se rompio todo\n");
        //write_out(PROMPT_START);
        exit_pcs(ERROR);
    }

    //write_out(PROMPT_START);
    exit_pcs(EXIT);
}

int wait(int argc, char ** argv){
    return create_process(&wait_dummy, "wait", argc, argv);
}

//TODO aplicar pipes, donde usa argv deberia se el input
void wc_dummy(int argc, char ** argv){
    
    char buffer[BUFFER_SIZE];
    int n;
    int line_count = 0;

    while ((n = read_input(buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buffer[i] == '\n') line_count++;
        }
    }

    char number[12];
    uintToBase(line_count, number, 10);
    write_out("Cantidad de lineas: ");
    write_out(number);
    write_out("\n");

    exit_pcs(EXIT);
}

int wc(int argc, char ** argv){
    return create_process(&wc_dummy, "WC", argc, argv);
}

//TODO aplicar pipes, donde usa argv deberia ser el input
void cat_dummy(int argc, char ** argv){
    for(int i = 0; argv[i] != NULL; i++){
        write_out(argv[i]);
        write_out(" ");
    }
    write_out("\n");
    exit_pcs(EXIT);
}

int cat(int argc, char ** argv){
    return create_process(&cat_dummy, "cat", argc, argv);
}

//TODO aplicar pipes, donde dice argv deberia ser el input
void filter_dummy(int argc, char ** argv){
    for(int i = 0; argv[i] != NULL; i++){
        for(int j = 0; argv[i][j] != '\0'; j++){
            if(argv[i][j] == 'a' || argv[i][j] == 'e' || argv[i][j] == 'i' || argv[i][j] == 'o' || argv[i][j] == 'u' 
            || argv[i][j] == 'A' || argv[i][j] == 'E' || argv[i][j] == 'I' || argv[i][j] == 'O' || argv[i][j] == 'U'){
                char vocal[2] = {argv[i][j], '\0'};
                write_out(vocal);
            }
        }
    }
    write_out("\n");
    exit_pcs(EXIT);
}

int filter(int argc, char ** argv){
    return create_process(&filter_dummy, "filter", argc, argv);
}

void mvar_dummy(int argc, char ** argv){
    write_out("Tdv no hay nada aca.\n");
    exit_pcs(EXIT);
}

int mvar(int argc, char ** argv){
    return create_process(&mvar_dummy, "mvar", argc, argv);
}