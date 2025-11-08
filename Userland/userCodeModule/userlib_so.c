#include "userlib_so.h"
#include "include/rand.h"

int shell_pid;
int idle_pid;

void estrellita_bg(){
    if(!foreground){
           write_out(PROMPT_START);
    }
}

void argc_1(int argc){
    if (argc != 1){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 1 solo argumento.\n");
        estrellita_bg();
        exit_pcs(ERROR);    
    }
    return;
}

// Crear la memoria
void create_mm(){
    _create_mm();
}

// Ocupa espacio en la memoria
void alloc_dummy(int argc, char ** argv){

    argc_1(argc);

    int size = char_to_int(argv[0]);

    _alloc(size);
    
    exit_pcs(EXIT);
}

int alloc(int argc, char ** argv){
    return create_process(&alloc_dummy, "alloc", argc, argv);
}

// Libera espacio de la memoria
void free_dummy(int argc, char ** argv){

    argc_1(argc);

    _free((void*) argv[0]);
    
    exit_pcs(EXIT);
}

int free(int argc, char ** argv){
    return create_process(&free_dummy, "free", argc, argv);
}

// Llena el array con los datos block_count, free_space, y used_space 
void status_count_dummy(int argc, char ** argv){

    if (argc != 0) {
        write_out("No tenias que mandar argumentos para este comando.\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }

    int status[3];
    _status_count(status);

    write_out("\n=== Estado del sistema de memoria ===\n");
    write_out("Bloques totales: ");
    char s0_str[21];
    int_to_str(status[0], s0_str);
    write_out(s0_str);
    write_out("\nBloques usados: ");
    char s1_str[21];
    int_to_str(status[1], s1_str);
    write_out(s1_str);
    write_out("\nBloques libres: ");
    char s2_str[21];
    int_to_str(status[2], s2_str);
    write_out(s2_str);
    write_out("\n");
    
    estrellita_bg();
    exit_pcs(EXIT);
}

int status_count(int argc, char ** argv){
    return create_process(&status_count_dummy, "status count", argc, argv);
}

int create_process(void * rip , const char *name, int argc, char *argv[]){
    return _create_process(rip, name, argc, argv, NULL);
}

int create_process_piped(void * rip , const char *name, int argc, char *argv[], uint64_t* fds){
    return _create_process(rip, name, argc, argv, fds);
}

void kill_dummy(int argc, char ** argv){

    argc_1(argc);

    int toKill = char_to_int(argv[0]);
    if(toKill == shell_pid){
        write_out("Para matar la shell tenes que usar Exit. \n");
        estrellita_bg();
        exit_pcs(EXIT);
    }
    if(toKill == idle_pid){
        write_out("No te podemos permitir matar el idle ¯\\_(ツ)_/¯\n");
        estrellita_bg();
        exit_pcs(EXIT);
    }
    if(toKill == _get_pid()){
        write_out("Kill al kill ?? ... okay\n");
        estrellita_bg();
        exit_pcs(EXIT);
    }
    if(kill_from_shell == 1){
        kill_from_shell = 0;
        write_out("Chau chau al proceso de pid: ");
        write_out(argv[0]);
        write_out("\n");
    }

    int ret = _kill_process(toKill); 
    char k_str[21];
    int_to_str(toKill, k_str);
    if(ret == ERROR){
        write_out("... O no\nEl pid ");
        write_out(k_str);
        write_out(" no es valido, asique no podemos matar a ningun proceso de ese pid... bobo.\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }
    if(ret==SECOND_ERROR){
        write_out("El proceso de pid ");
        write_out(k_str);
        write_out(" ya esta muerto.\n");
        exit_pcs(ERROR);
    }

    estrellita_bg();
    exit_pcs(EXIT);
}

//recibe el pid de a quien matar por argv
int kill_process(int argc, char ** argv){
    return create_process(&kill_dummy, "kill", argc, argv);
}

//cada proceso cuando termina se debe "matar" a si mismo, osea dejar marcado con KILLED
void exit_pcs(int ret){
    
    int pid;
    if(bye_shell){
        pid = shell_pid;
    }
    else{
        pid = _get_pid(); 
    }
    char pid_str[21];
    int_to_str(pid, pid_str);

    if(ret == ERROR){
        write_out("El proceso de pid ");
        write_out(pid_str);
        write_out(" cerro con error\n");
    }

    int ret_del_kill = _kill_process(pid);
    if(ret_del_kill == ERROR){
        write_out("Hubo un error en el exit ya que el pid ");
        write_out(pid_str);
        write_out(" no es valido... big problem, no deberias llegar a aca nunca\n");
        estrellita_bg();
        exit_pcs(EXIT);
    }
}

void block_process_dummy(int argc, char ** argv){

    argc_1(argc);

    int pid = char_to_int(argv[0]);
    if(pid == shell_pid){
        write_out("Por ahora la shell no es bloqueante, asi que... adios\n");
    }
    if(pid == idle_pid){
        write_out("Bloquear el idle... no sos muy inteligente\n");
    }
    if(pid == get_pid()){
        write_out("Que haces loco, nos vas a meter en problemas raja de aca.\n");
        estrellita_bg();
        exit_pcs(ERROR); 
    }

    int ret = _block_process(pid);

    if(ret == ERROR){
        write_out("Este pid no es valido\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){
        write_out("Este pid ya estaba muerto\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }

    estrellita_bg();
    exit_pcs(EXIT);
}

int block_process(int argc, char ** argv){
    return create_process(&block_process_dummy, "block", argc, argv);
}

void unblock_process_dummy(int argc, char ** argv){
   
    argc_1(argc);
    int pid = char_to_int(argv[0]);

    if(pid == idle_pid){
        write_out("Que estas haciendo? esto no sirve de nada, va a volver estar blocked cuando hagas ps.\n");
    }
    int ret = _unblock_process(pid);

    if(ret == ERROR){ 
        write_out("Este pid no es valido\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){ 
        write_out("Este proceso no esta bloqueado, asique no lo podemos desbloquear\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }

    estrellita_bg();
    exit_pcs(EXIT);
}

int unblock_process(int argc, char ** argv){
    return create_process(&unblock_process_dummy, "unblock", argc, argv);
}

void get_proc_list_dummy(int argc, char ** argv){

    ProcessInfo* list = _get_proc_list();
    if (list == NULL) {
        write_out("Error, no se pudo obtener la lista de procesos.\n");
        estrellita_bg();
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

        char pid_str[21];
        int_to_str(p->pid, pid_str);
        write_out(pid_str);
        write_out("\t");
        write_out(p->name);
        write_out("\t");
        write_out(p->state);
        write_out("\t");
        if (p->parentPid == (uint64_t)-1) write_out("-1");
        else {
            char ppid_str[21];
            int_to_str(p->parentPid, ppid_str);
            write_out(ppid_str);
        }
        write_out("\t0x");
        const char hex_chars[] = "0123456789ABCDEF";
        char hex_buffer[17];  // solo los dígitos hex, sin prefijo
        for (int i = 0; i < 16; i++) {
            int shift = (15 - i) * 4;
            hex_buffer[i] = hex_chars[(p->rsp >> shift) & 0xF];
        }
        hex_buffer[16] = '\0';
        // Saltar ceros a la izquierda
        int start = 0;
        while (hex_buffer[start] == '0' && start < 15) {
            start++;
        }
        if (start == 16) start = 15;    // Si todo era 0, mostrar un solo 0
        write_out(&hex_buffer[start]);
        write_out("\t");
        write_out(p->my_prio);
        write_out("\t");
        char chi_str[21];
        int_to_str(p->childrenAmount, chi_str);
        write_out(chi_str);
        write_out("\t");
        char fdc_str[21];
        int_to_str(p->fileDescriptorCount, fdc_str);
        write_out(fdc_str);
        write_out("\n");
    }

    write_out("-------------------------------------------------------------\n");

    _free(list);
    estrellita_bg();
    exit_pcs(EXIT);
}

int get_proc_list(int argc, char ** argv){
    return create_process(&get_proc_list_dummy, "ps", argc, argv);
}

//ésta no es proceso, es built-in porq sino devolveria su propio pid
int get_pid(){
    return _get_pid();
}

void yield_dummy(int argc, char ** argv){
    _yield();
    
    exit_pcs(EXIT);
}

int yield(int argc, char ** argv){
    return create_process(&yield_dummy, "yield", argc, argv);
}

void be_nice_dummy(int argc, char ** argv){

    if (argc != 2){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 2 argumentos.\n");
        estrellita_bg();
        exit_pcs(ERROR);    
    }

    int pid = char_to_int(argv[0]);
    int new_prio = char_to_int(argv[1]);

    int ret = _be_nice(pid, new_prio);

    if(ret == ERROR){ 
        write_out("Este pid no es valido\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }
    if(ret == SECOND_ERROR){
        write_out("Epa, intentaste cambiar la prioridad del idle, nonono\n");
        estrellita_bg();
        exit_pcs(EXIT);
    }
    if(ret == -3){
        write_out("Mandaste una prioridad inexistente, porfavor acordate que las prioridades son de 0 a 4, siendo 0 la mayor\n");
        estrellita_bg();
        exit_pcs(ERROR);
    }

    estrellita_bg();
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
  
    argc_1(argc);
    int loop_fg=0;
    if(foreground){
        loop_fg=1;
    }
    int pid = get_pid(); //agarro mi priopio pid
    int cloop=0;
    int tiempo = char_to_int(argv[0]);   
    char pid_str[21];
    int_to_str(pid, pid_str);
    while (1){
        if(loop_fg){
            write_out(PROMPT_START);
        }
        write_out("Hola soy el loop, y mi pid es: ");
        write_out(pid_str);
        write_out(".\t Esta es mi vuelta ");
        char c_str[21];
        int_to_str(cloop, c_str);
        write_out(c_str);
        cloop++;
        write_out(".\n");
        sleep(tiempo, 0);
    }
}

int loop(int argc, char ** argv){
    return create_process(&loop_dummy,"loop", argc, argv);
}

void wc_dummy(int argc, char ** argv){
    char buffer[2];
    int line_count = 1;
    buffer[1] = '\0';

    while (1) {
        // Se llama a read para leer de STDIN (o sea FD 0)
        int bytes_read = read(buffer, 1);
        char c = buffer[0];

        if (bytes_read > 0) {
            if (c == '\n') {             // Conteo de lineas
                line_count++;
            }
            write_out(buffer);
        } 
        else {
            // EOF (el pipe se cerro) o hubo un error
            if (bytes_read == 0) {
                char number_str[12];
                uintToBase(line_count, number_str, 10);
                write_out("\nCantidad de lineas: ");
                write_out(number_str);
                write_out("\n");
            }
            break;
        }
    }

    exit_pcs(EXIT);
}

int wc(int argc, char ** argv){
    return create_process(&wc_dummy, "WC", argc, argv);
}

void cat_dummy(int argc, char ** argv){
    
    char line_buffer[BUFFER_SIZE]; 
    int line_index = 0; 
    char echo_buffer[2];
    char read_buffer[1];
    echo_buffer[1] = '\0';
    
    while (1) {
        // Llama a read() para leer de STDIN (o sea FD 0)
        int bytes_read = read(read_buffer, 1);

        if (bytes_read > 0) {
            char c = read_buffer[0];

            if (c == '\b') { // Backspace
                if (line_index > 0) {
                    line_index--;
                    // Imprime el backspace para mover el cursor
                    echo_buffer[0] = '\b'; 
                    write_out(echo_buffer);
                }
                continue;
            }

            echo_buffer[0] = c;
            write_out(echo_buffer);
            
            if (line_index < BUFFER_SIZE - 1) {
                line_buffer[line_index++] = c;
            }

            // Si se toco 'Enter', imprimir linea completa
            if (c == '\n') {
                line_buffer[line_index] = '\0';
                write_out(line_buffer);
                line_index = 0; 
            }

        } else {
            // EOF o error: si hubo datos parciales, imprimirlos antes de terminar
            if (line_index > 0) {
                line_buffer[line_index] = '\0';
                write_out("\n");
                write_out(line_buffer);
                write_out("\n");
            }
            break;
        }
    }
    
    exit_pcs(EXIT);
}

int cat(int argc, char ** argv){
    return create_process(&cat_dummy, "cat", argc, argv);
}

void filter_dummy(int argc, char ** argv){
    
    char line_buffer[BUFFER_SIZE];
    char filtered_buffer[BUFFER_SIZE];
    int line_index = 0;

    char read_buffer[1];
    char echo_buffer[2];  
    echo_buffer[1] = '\0';
    
    while (1) {
        int bytes_read = read(read_buffer, 1);

        if (bytes_read > 0) {
            char c = read_buffer[0];

            if (c == '\b') { // Backspace
                if (line_index > 0) {
                    line_index--;
                    
                    // Imprime el backspace para mover el cursor
                    echo_buffer[0] = '\b'; 
                    write_out(echo_buffer);
                }
                continue;
            }

            echo_buffer[0] = c;
            write_out(echo_buffer);

            if (line_index < BUFFER_SIZE - 1) {
                line_buffer[line_index++] = c;
            }

            // Si la tecla fue Enter, procesar la línea
            if (c == '\n') {
                int filtered_index = 0;
                
                for (int i = 0; i < line_index; i++) {
                    char ch = line_buffer[i];
                    if(ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' ||
                       ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U')
                    {
                        filtered_buffer[filtered_index++] = ch;
                    }
                }
                filtered_buffer[filtered_index] = '\0';
                
                // Imprime el resultado
                write_out(filtered_buffer);
                write_out("\n");
                
                line_index = 0; 
            }

        } else {
            // EOF o error: si hubo datos parciales, procesarlos e imprimir
            if (line_index > 0) {
                int filtered_index = 0;
                for (int i = 0; i < line_index; i++) {
                    char ch = line_buffer[i];
                    if(ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' ||
                       ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U')
                    {
                        filtered_buffer[filtered_index++] = ch;
                    }
                }
                filtered_buffer[filtered_index] = '\0';
                write_out(filtered_buffer);
                write_out("\n");
            }
            break;
        }
    }
    
    exit_pcs(EXIT);
}

int filter(int argc, char ** argv){
    return create_process(&filter_dummy, "filter", argc, argv);
}

void msg_dummy(int argc, char ** argv){
    write_out("Arquitectura de Computadoras\n");
    exit_pcs(EXIT);
}

int msg(int argc, char ** argv){
    return create_process(&msg_dummy, "msg", argc, argv);
}

void writer_dummy(int argc, char ** argv) {
    // argv[0] = string con la letra que deberian imprimir
    srand(_get_pid());

    while (1) {
        // espera activa
        int delay = rand() % 2 + 1;
        sleep(delay, 0);

        // se espera a que este vacio
        _sem_wait("MVAR_EMPTY");

        // Escritura: escribimos directo al pipe con la syscall
        int pid_pipe = char_to_int(argv[1]);
        char buf[1]; buf[0] = argv[0][0];
        _pipe_write(pid_pipe, buf, 1);

        _sem_post("MVAR_FULL");
    }

    exit_pcs(EXIT);
}

static const uint32_t colors[5] = {
        0xFFFFFF, // blanco
        0x000000, // negro
        0xFF0000, // rojo
        0x00FF00, // verde
        0xFFFF00  // amarillo
};

uint32_t get_color_from_int(int n) {

    int index = n % 5;
    if (index < 0) index += 5; // por si n es negativo

    return colors[index];
}

void reader_dummy(int argc, char ** argv) {
    // argv[0] = reader id string (optional)
    srand(_get_pid());

    while (1) {
        int delay = rand() % 2 + 1;
        sleep(delay, 0);

        _sem_wait("MVAR_FULL");

        char buf[2];
        buf[1] = '\0';
        int bytes = _pipe_read(char_to_int(argv[1]), buf, 1);

        if (bytes > 0) {
            _print_color(buf, 1, get_color_from_int(char_to_int(argv[0])), 0x01233E);
        } 
        else {
            write_out("[dbg] reader EOF\n");
            break;
        }

        _sem_post("MVAR_EMPTY");
    }

    exit_pcs(EXIT);
}

void mvar_dummy(int argc, char ** argv){
    // args: <n_writers> <n_readers>
    if (argc != 2) {
        write_out("Cantidad de argumentos incorrecta, el uso correcto es mvar <n_writers> <n_readers>\n");
        exit_pcs(ERROR);
    }

    int n_writers = char_to_int(argv[0]);
    int n_readers = char_to_int(argv[1]);

    if (n_writers <= 0 || n_readers <= 0) {
        write_out("Parametros invalidos. Ambos deben ser mayor a 0.\n");
        exit_pcs(ERROR);
    }

    // inicializa los semaforos: SPACE=1 (vacio), ITEMS=0 (no hay items)
    _sem_open_init("MVAR_EMPTY", 1);
    _sem_open_init("MVAR_FULL", 0);

    // crea el pipe
    int pipe_id = _pipe_create_named("MVAR_PIPE");
    if(pipe_id < 0){
        write_out("Hubo un error al crear el pipe.\n");
        exit_pcs(ERROR);
    }


    // spawnea los writers
    for (int i = 0; i < n_writers; i++) {
    char letter[2];
    letter[0] = 'A' + (i % 26);
    letter[1] = '\0';
    char pipebuf[12];
    int_to_str(pipe_id, pipebuf);
    char *wargv[3];
    wargv[0] = letter;
    wargv[1] = pipebuf;
    wargv[2] = NULL;

        uint64_t fds_w[2];
        fds_w[0] = 0;        // STDIN
        fds_w[1] = pipe_id;  // STDOUT -> pipe (write)

        create_process_piped(&writer_dummy, "mvar_writer", 2, wargv, fds_w);
    }

    // spawnea los readers
    for (int j = 0; j < n_readers; j++) {
    char id_str[8];
    int_to_str(j+1, id_str);

    char pipebuf_r[12];
    int_to_str(pipe_id, pipebuf_r);

    char *rargv[3];
    rargv[0] = id_str;
    rargv[1] = pipebuf_r;
    rargv[2] = NULL;

        uint64_t fds_r[2];
        fds_r[0] = pipe_id;  // STDIN -> pipe (read)
        fds_r[1] = 1;        // STDOUT

        create_process_piped(&reader_dummy, "mvar_reader", 2, rargv, fds_r);
    }

    //El proceso principal debe terminar inmediatamente después de crear los lectores y escritores.​
    write_out("Se crearon exitosamente los procesos para el mvar. Espera a que se impriman los caracteres.\n");
    exit_pcs(EXIT);
}
int mvar(int argc, char ** argv){
    return create_process(&mvar_dummy, "mvar", argc, argv);
}

void sem_open_init_dummy(int argc, char ** argv){
    
    if(argc!=2){
        write_out("No mandaste la cantidad de argumentos correcta. Intentalo otra vez, pero con 2 argumentos.\n");
        exit_pcs(ERROR);
    }

    int val = char_to_int(argv[1]);
    int ret = _sem_open_init(argv[0], val);
    if(ret==ERROR){
        write_out("El semaforo ");
        write_out(argv[0]);
        write_out(" ya existe\n");
        exit_pcs(ERROR);
    }
    if(ret==SECOND_ERROR){
        write_out("No hay espacio para más semaforos");
        exit_pcs(ERROR);
    }

    exit_pcs(EXIT);
}

int sem_open_init(int argc, char ** argv){
    return create_process(&sem_open_init_dummy, "sem open/init", argc, argv);
}

void sem_wait_dummy(int argc, char ** argv){

    argc_1(argc);
    int ret = _sem_wait(argv[0]);
    if(ret == ERROR){
        write_out("El semaforo ");
        write_out(argv[0]);
        write_out(" no existe\n");
        exit_pcs(ERROR);
    }

    exit_pcs(EXIT);
}

int sem_wait(int argc, char ** argv){
    return create_process(&sem_wait_dummy, "sem wait", argc, argv);
}

void sem_post_dummy(int argc, char ** argv){

    argc_1(argc);

    int ret = _sem_post(argv[0]);
    if(ret == ERROR){
        write_out("El semaforo ");
        write_out(argv[0]);
        write_out(" no existe\n");
        exit_pcs(ERROR);
    }

    exit_pcs(EXIT);
}

int sem_post(int argc, char ** argv){
    return create_process(&sem_post_dummy, "sem wait", argc, argv);
}

void sem_close_dummy(int argc, char ** argv){

    argc_1(argc);
    int ret = _sem_close(argv[0]);
    if(ret == ERROR){
        write_out("El semaforo ");
        write_out(argv[0]);
        write_out(" no existe\n");
        exit_pcs(ERROR);
    }

    exit_pcs(EXIT);

}

int sem_close(int argc, char ** argv){
    return create_process(&sem_close_dummy, "sem wait", argc, argv);
}