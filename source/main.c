/*compile with hd.c*/
#include "hd_h.h"

int main()
{
    InputBuffer* input_buffer = new_input_buffer(); // Создание буфера для получения команд от пользователя 

    while(1)
    {
        print_prompt(); // инвайт на ввод пользователя
        read_input(input_buffer); // чтение 

        if(strcmp(input_buffer->buffer, ".exit") == 0)
        {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        } 
    } 
    return 0;
}

