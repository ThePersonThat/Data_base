#ifndef _HD_H_
    #define _HG_H

#include <sys\types.h>

typedef struct 
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

InputBuffer *new_input_buffer();

#endif