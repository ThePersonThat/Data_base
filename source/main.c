#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "mtcom.h"
#include "operator.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Must supply a database filename.\n");
		exit(EXIT_FAILURE);
	}

	char* filename = argv[1];
	Table* table = db_open(filename);
   
    InputBuffer* input_buffer = new_input_buffer();
    
    while (1)
    {
        print_promt();
        read_input(input_buffer);

        if(input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table)) 
            {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command '%s'\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error. Could not parse statment.\n");
                continue;
			case PREPARE_STRING_TOO_LONG:
				printf("String is too long.\n");
				break;
			case PREPARE_NEGATIVE_ID:
				printf("ID must be positive.\n");
				break;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);

            continue;
        }

        switch (execute_statement(&statement, table))
        {
            case EXECUTE_SUCCCESS:
                printf("Executed.\n");
                break;
			case EXECUTE_DUPLICATE_KEY:
				printf("Error: Duplicate key.\n");
				break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
        }
    }
}