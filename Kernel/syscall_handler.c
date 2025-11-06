#include <keyboard.h>
#include <lib.h>
#include <stdint.h>
#include <syscall_handler.h>
#include <videoDriver.h>
#include <time.h>
#include <registers.h>
#include <sound.h>
#include "include/memory_manager.h"
#include "include/proc.h"
#include "include/scheduler.h"
#include "include/pipes.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern int d_flag;
extern int c_flag;


void syscall_handler(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t rax) {
  switch (rax) {

  case (0x00):
    sys_read(rdi, rsi, rdx);
    break;

  case (0x01):
    sys_write(rdi, rsi, rdx);
    break;

  case (0x77):
    sys_draw(rdi, rsi, rdx, rcx, r8);
    break;

  case (0x83):
    sys_screenData(rdi, rsi, rdx, rcx);
    break;

  case (0x4e):
    sys_gettimeofday(rdi, rsi, rdx);
    break;
    
  case (0x23):
    sys_sleep(rdi, rsi);
    break;  

  case (0x78):
    sys_registers(rdi);
    break;

  case (0x93):
    sys_changeSize(rdi, rsi);
    break;

  case (0x84):
    sys_speak(rdi, rsi);
    break;

  case (0x30):
    sys_create_mm();
    break;

  case (0x31):
    sys_alloc(rdi);
    break;

  case(0x32):
    sys_free(rdi);
    break;

  case (0x33):
    sys_status_count(rdi);
    break;

  case (0x34):
    sys_yield();
    break;

  case (0x35):
    sys_be_nice(rdi, rsi);
    break;
    
  case (0xA0):
    sys_create_process(rdi, rsi, rdx, rcx, r8);
    break;
  
  case (0xA1):
    sys_kill_process(rdi);
    break;
  
  case (0xA2):
    sys_block_process(rdi);
    break;
  
  case (0xA3):
    sys_unblock_process(rdi);
    break;
  
  case (0xA4):
    sys_get_proc_list();
    break;

  case (0XA5):
    sys_get_pid();
    break;

  case (0xA6):
    sys_wait(rdi, rsi, rdx);
    break;

  case (0xA7):
    sys_sem_open_init(rdi, rsi);
    break;
  
  case (0xA8):
    sys_sem_wait(rdi);
    break;

  case (0xA9):
    sys_sem_post(rdi);
    break;

  case (0xAA):
    sys_sem_close(rdi);
    break;

  case (0xAB):
    sys_pipe_create_anonymous(rdi);
    break;

  case (0xAC):
    sys_pipe_create_named(rdi);
    break;

  case (0xAD):
    sys_pipe_close(rdi);
    break;

  case (0xAE): 
    sys_pipe_write(rdi, rsi, rdx);
    break;

  case (0xAF):
    sys_pipe_read(rdi, rsi, rdx);;
    break;

  case (0xB0):
    sys_get_shell();
    break;

  case (0xB1):
    sys_get_idle();
    break;
    
  }
}

void sys_write(uint64_t fd, uint64_t message, uint64_t length) {
  int current_pid = get_pid();
  if (!is_pid_valid(current_pid)) return;
  PCB* pcb = processTable[current_pid];

  if (fd >= MAX_FD) return;
  int real_fd = pcb->fileDescriptors[fd];

  switch (real_fd) {
    case (STDOUT): // El FD real es 1
      printCant((char *)message, length);
      break;

    case (STDERR): // El FD real es 2
      printColorCant((char *)message, length, ERRCOLORFONT, ERRCOLORBACK);
      break;
    
    default:
      // Si no es STDOUT o STDERR asumimos que es un PIPE
      if (real_fd > STDERR) {
          sys_pipe_write(real_fd, message, length);
      }
    break;
  }
}

int sys_read(uint64_t fd, uint64_t buffer, uint64_t length) {
    
    int current_pid = get_pid();
    if (!is_pid_valid(current_pid)) return -1;
    PCB* pcb = processTable[current_pid];

    if (fd >= MAX_FD) return -1;
    int real_fd = pcb->fileDescriptors[fd]; 

    if (real_fd == STDIN) {
        // El FD real es 0 (STDIN/Teclado)
        return read_chars(real_fd, (char*)buffer, length);
    } 
    
    if (real_fd > STDERR) { 
        // El FD real es un pipe
        return sys_pipe_read(real_fd, buffer, length);
    }

    return -1;
}

int read_chars(int fd, char *buffer, int length) {
  if (fd != STDIN) {
      int chars_read = 0;
      for (int i = 0; i < length; i++) {
          buffer[i] = read_key(fd);
          if (buffer[i] == 0) {
              return chars_read; 
          }
          chars_read++;
      }
      return chars_read;
  }
  int chars_read = 0;
  while (chars_read < length) {
      
      sem_wait("sem_stdin");

      char c = read_key(fd);

      if (c == 0) continue;

      // (Ctrl+D)
      if (d_flag == 1) {
          d_flag=0;
          buffer[chars_read++] = '\x04';
          return chars_read;
      }
      
      if (c_flag == 1) { // Ctrl+C
          c_flag=0;
          buffer[chars_read++] = '\x03';
          return chars_read; 
      }
      
      buffer[chars_read++] = c;

      if (c == '\n') {
          return chars_read;
      }
  }
  return chars_read;
}

void sys_draw(uint64_t bitmap, uint64_t hexColor, uint64_t height, uint64_t init_x, uint64_t init_y){
  printBitmap((uint16_t *) bitmap, hexColor, height, init_x, init_y);
}

void sys_screenData(uint64_t screenHeight, uint64_t screenWidth, uint64_t fontSize, uint64_t drawSize){
  getScreenData((uint16_t *) screenHeight, (uint16_t *) screenWidth, (uint8_t *) fontSize, (uint8_t *) drawSize);
}

void getScreenData(uint16_t * screenHeight, uint16_t * screenWidth, uint8_t * fontSize, uint8_t * drawSize){
  *screenHeight=getScreenHeight();
  *screenWidth=getScreenWidth();
  *fontSize=getFontSize();
  *drawSize=getDrawSize();
}

void sys_sleep(uint32_t  cant,uint32_t  unidad){
  sleep(cant, unidad); 
}

void sys_gettimeofday(uint64_t hrs, uint64_t min, uint64_t seg){
  printTime((int*) hrs, (int*)min, (int *)seg);
 }

int sys_registers(uint64_t regs){
    return getRegs((uint64_t*) regs);
}

void sys_changeSize(uint8_t newSize, uint8_t fd){
  switch(fd){
    case(1):
      changeFontSize(newSize);
      break;

    case(2):
      changeDrawSize(newSize);
      break;
      
    default:
      break;
  }
}

void sys_speak(uint64_t frequence, uint64_t duration){
    beep((uint32_t) frequence, (int) duration);
}

/*================================================================================================================================
SISTEMAS OPERATIVOS
================================================================================================================================*/

void sys_create_mm(){
    create_mm();
}

void *sys_alloc(uint64_t size){
    return alloc(size);
}

void sys_free(uint64_t address){
    free((void *) address);
}

void sys_status_count(uint64_t status_out){
  status_count((uint32_t *) status_out);
}

void sys_yield(){
  yield();
}

int sys_be_nice(uint64_t pid, uint64_t newPrio){
    return be_nice(pid, newPrio);
}

int sys_create_process(uint64_t rip, uint64_t name, uint64_t argc, uint64_t argv, uint64_t fds){
  return create_process((void *) rip, (char *) name, (int) argc, (char **) argv, (uint64_t*) fds);
}

int sys_kill_process(uint64_t pid){
  return kill_process(pid);
}

int sys_block_process(uint64_t pid){
   return block_process(pid);
}

int sys_unblock_process(uint64_t pid){
  return unblock_process(pid);
}

ProcessInfo* sys_get_proc_list(){
  return get_proc_list();
}

int sys_get_pid(){
  return get_pid();
}

int sys_wait(uint64_t target_pid, uint64_t my_pid, uint64_t name){
  return wait(target_pid, my_pid, (char *)name);
}

int sys_sem_open_init(uint64_t name, uint64_t value){
  return sem_open_init((char *)name, value);
}

int sys_sem_wait(uint64_t name){
  return sem_wait((char *)name);
}

int sys_sem_post(uint64_t name){
  return sem_post((char *)name);
}

int sys_sem_close(uint64_t name){
  return sem_close((char *)name);
}

//PIPES

int sys_pipe_create_anonymous(uint64_t pipe_ids){
  return pipe_create_anonymous((int *) pipe_ids);
}

int sys_pipe_create_named(uint64_t name){
  return pipe_create_named((const char*) name);
}

int sys_pipe_close(uint64_t pipe_id){
  return pipe_close((int) pipe_id);
}

int sys_pipe_write(uint64_t pipe_id, uint64_t buffer, uint64_t count){
  return pipe_write((int) pipe_id, (const char*) buffer, (int) count);
}

int sys_pipe_read(uint64_t pipe_id, uint64_t buffer, uint64_t count){
  return pipe_read((int) pipe_id, (char*) buffer, (int) count);
}


int sys_get_shell(){
  return get_shell_pid();
}

int sys_get_idle(){
  return get_idle_pid();
}
