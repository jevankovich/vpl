#ifndef READLINE_H
#define READLINE_H

#include <stddef.h>

// readline reads a single line of input. If more than one line is read, the
// pointer arguments provide enough state to restart the next line in the
// buffer. buf is automatically resized to fit the line.
//
// buf is a pointer to a char buffer. If this is NULL, a new buffer is
// allocated
//
// len is a pointer to the size of the buffer. The size is automatically
// updated as the buffer grows
//
// readlen is a pointer that indicates the length of the buffer that is valid
// data. This is used to restore the state of the reader on successive calls
//
// linelen is the length of the first line present in the buffer. This is also
// used to restore the state of the reader
//
// On successive calls, the returned value should be the value passed as
// linelen
size_t readline(char **buf, size_t *len, size_t *readlen, size_t linelen);
#endif
