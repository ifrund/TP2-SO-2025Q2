#ifndef _PIPES_H_
#define _PIPES_H_

#include <stdint.h>

#define STDIN 0
#define STDOUT 1
#define STDERROR 2

#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

// Creates or opens a pipe with the given 'name'. Returns a pipe identifier on success, or -1 on failure.
int pipe_create_named(const char* name);

// Creates an anonymous pipe and fills 'pipe_ids' with the read and write ends' identifiers. Returns 0 on success, or -1 on failure.
int pipe_create_anonymous(int pipe_ids[2]);

// Closes the pipe identified by 'pipe_id'.
int pipe_close(int pipe_id, int mode);

// Writes up to 'count' bytes from 'buffer' into the pipe identified by 'pipe_id'.
int pipe_write(int pipe_id, const char* buffer, int count);

// Reads up to 'count' bytes from the pipe identified by 'pipe_id' into 'buffer'.
int pipe_read(int pipe_id, char* buffer, int count);

void pipe_init();

// Register that a process has opened one end of the pipe. mode is PIPE_READ_END or PIPE_WRITE_END.
int pipe_register(int pipe_id, int mode);

#endif // _PIPES_H_