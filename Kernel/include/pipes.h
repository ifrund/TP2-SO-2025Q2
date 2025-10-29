#ifndef _PIPES_H_
#define _PIPES_H_

#include <stdint.h>

// Creates or opens a pipe with the given 'name'. Returns a pipe identifier on success, or -1 on failure.
int pipe_open(const char* name);

// Closes the pipe identified by 'pipe_id'.
int pipe_close(int pipe_id);

// Writes up to 'count' bytes from 'buffer' into the pipe identified by 'pipe_id'.
int pipe_write(int pipe_id, const char* buffer, int count);

// Reads up to 'count' bytes from the pipe identified by 'pipe_id' into 'buffer'.
int pipe_read(int pipe_id, char* buffer, int count);

#endif // _PIPES_H_