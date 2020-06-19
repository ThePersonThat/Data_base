#include <string.h>
#include <stdlib.h>

#include "mtcom.h"

MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table)
{
    if(strcmp(input_buffer->buffer, ".exit") == 0)
    {
		db_close(table);
		close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);        
    }
	else if (strcmp(input_buffer->buffer, ".constants") == 0)
	{
		printf("Constants: \n");
		print_constants();
		return META_COMMAND_SUCCESS;
	}
	else if (strcmp(input_buffer->buffer, ".btree") == 0)
	{
		printf("Tree: \n");
		print_tree(table->pager, 0, 0);
		return META_COMMAND_SUCCESS;
	}
    else 
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}