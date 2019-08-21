#ifndef _HD_H_
    #define _HG_H
#include <stddef.h>
#include <sys\types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
    char* buffer; // указатель на буфер, который будет исользоваться для текста
    size_t buffer_length; // константный размер буфера
    ssize_t input_length; // размер буфера который ввел пользователь 
}InputBuffer;

InputBuffer *new_input_buffer(); // инизиализация буфера
void print_prompt(); // инвайт на ввод пользователя
void read_input(InputBuffer* input_buffer); // чтение
void close_input_buffer(InputBuffer* input_buffer); // очистка памяти

#endif