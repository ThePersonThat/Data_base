#ifndef INPUT_H
#define INPUT_H

#include <BaseTsd.h>

typedef struct 
{
    char* buffer;
    size_t buffer_lenght;
    SSIZE_T input_lenght;
} InputBuffer;

InputBuffer* new_input_buffer();
void print_promt();
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);


#endif