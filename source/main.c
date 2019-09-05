/*compile with hd.c*/
#include "hd_h.h"

int main()
{
    InputBuffer* input_buffer = new_input_buffer(); // Создание буфера для получения команд от пользователя 
    Table* table = new_table();

    while(1)
    {
        print_prompt(); // инвайт на ввод пользователя
        read_input(input_buffer); // чтение 

        if(input_buffer->buffer[0] == '.') // сравниваем на ввод мета команд
        {
            switch (do_meta_command(input_buffer)) // какая именно мета команда используется 
            {
            case(META_COMMAND_HELP_WRONG):
                printf("Wrong the string. Expamle: \".help meta\" - for meta-commands or \".help oper\" - for operators.\n");
                continue;

            case(META_COMMAND_HELP_LONG):
                printf("The arguments are too long.\n");
                continue;

            case (META_COMMAND_SUCCESS): // на случай правильного ввода
                continue;

            case (META_COMMAND_UNRECOGNIZED_COMMAND): // на случай неправильного ввода
                printf("Unrecognized command '%s' \n", input_buffer->buffer);
                continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) // проверка какой был оператор введен
        {
            case (PREPARE_NEGATIVE_ID):
                printf("ID must be positive"); // на случай отрицательного id
                continue;

            case (PREPARE_STRING_TOO_LONG): // на случай слишком длинной строки
                printf("The string is too long");
                continue;

            case (PREPARE_SUCCESS): // на случай правильного ввода
                break;

            case (PREPARE_SYNTAX_ERROR): // на случай неправильного ввода
                printf("Syntax error. Could not parse statement.\n");
                continue;

            case (PREPARE_UNRECOGNIZED_STATEMENT): 
                printf("Unrecognized keyword at start of '%s'. \n", input_buffer->buffer);
                continue;
        }

        switch (execute_statement(&statement, table)) // выполнение оператора 
        {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        case (EXECUTE_TABLE_FULL):
            printf("Error: Table full.\n");
            break;
        }
    }  
    return 0;
}

