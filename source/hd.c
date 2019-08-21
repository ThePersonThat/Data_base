#include "hd_h.h"

InputBuffer* new_input_buffer()
{
    InputBuffer *input_buffer = (InputBuffer *) malloc(sizeof(InputBuffer)); // выделение памяти под структуру
    input_buffer->buffer = NULL; // обноление уазателя под ввод пользователя 
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void print_prompt()
{
     printf("db > ");
}

void read_input(InputBuffer* input_buffer)
{
    //ssize_t getline(char **lineptr, size_t *n, FILE *stream);
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin); // количество удачно прочитанных байтов

    if(bytes_read <= 0) 
    {
        printf("Error reading input\n");
        exit(EXIT_SUCCESS);
    }

    input_buffer->buffer_length = bytes_read - 1; // размер буфера введенным пользователем
    input_buffer->buffer[bytes_read - 1] = '\0'; // окончание пользовательской строки
}

void close_input_buffer(InputBuffer* input_buffer) 
{
    free(input_buffer->buffer); // очистка выделанного буфера
    free(input_buffer); // очистка структуры
}