#ifndef SHELL_H
#define SHELL_H

extern char PROMPT_START[];

int shell();
void process_key(char key);
void process_command(char *buffer);
void shift();
int check_shift();
void write_out(char *string);
void init_shell();
int read_input(char *buffer, int max_len);

#endif