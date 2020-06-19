#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "operator.h"
#include "constant.h"
#include "btree.h"

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;

const uint32_t USERNAME_OFFSET = 0 + size_of_attribute(Row, id);
const uint32_t EMAIL_OFFSET = size_of_attribute(Row, username) + size_of_attribute(Row, id);
const uint32_t ROW_SIZE = size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email);


PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    if(strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
		return prepare_insert(input_buffer, statement);
    }
    if(strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}


PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statment)
{
	statment->type = STATEMENT_INSERT;

	char* keyword = strtok(input_buffer->buffer, " ");
	char* id_string = strtok(NULL, " ");
	char* username = strtok(NULL, " ");
	char* email = strtok(NULL, " ");

	if (id_string == NULL || username == NULL || email == NULL)
		return PREPARE_SYNTAX_ERROR;

	int id = atoi(id_string);

	if (id < 0)
		return PREPARE_NEGATIVE_ID;

	if (strlen(username) > COLUMN_USERNAME_SIZE)
		return PREPARE_STRING_TOO_LONG;

	if (strlen(email) > COLUMN_EMAIL_SIZE)
		return PREPARE_STRING_TOO_LONG;

	statment->row_to_insert.id = id;
	strcpy(statment->row_to_insert.username, username);
	strcpy(statment->row_to_insert.email, email);

	return PREPARE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    case STATEMENT_SELECT:
        return execute_select(statement, table);
    }
}

ExecuteResult execute_insert(Statement* statement, Table* table)
{
	void* node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = (*leaf_node_num_cells(node));
	if (num_cells >= LEAF_NODE_MAX_CELLS)
		return EXECUTE_TABLE_FULL;

	Row* row_to_insert = &(statement->row_to_insert);
	uint32_t key_to_insert = row_to_insert->id;
	Cursor* cursor = table_find(table, key_to_insert);

	if (cursor->cell_num < num_cells)
	{
		uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
		if (key_at_index == key_to_insert)
			return EXECUTE_DUPLICATE_KEY;
	}

	leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

	free(cursor);
}

ExecuteResult execute_select(Statement* statement, Table* table)
{
	Cursor* cursor = table_start(table);

    Row row;
	while (!(cursor->end_of_table))
	{
		deserialize_row(cursor_value(cursor), &row);
		print_row(&row);
		cursor_advance(cursor);
	}

	free(cursor);

    return EXECUTE_SUCCCESS;
}

void print_row(Row* row)
{
    printf("%d %s %s\n", row->id, row->username, row->email);
}


void serialize_row(Row* source, void* destination)
{
    memcpy((void *)((uintptr_t)destination + ID_OFFSET), &(source->id), ID_SIZE);
	memcpy((void *)((uintptr_t)destination + USERNAME_OFFSET), &(source->username), USERNAME_SIZE);
	memcpy((void *)((uintptr_t)destination + EMAIL_OFFSET), &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination)
{
    memcpy(&(destination->id), (void *)((uintptr_t)source + ID_OFFSET), ID_SIZE);
    memcpy(&(destination->username), (void *)((uintptr_t)source + USERNAME_OFFSET), USERNAME_SIZE);
    memcpy(&(destination->email), (void *)((uintptr_t)source + EMAIL_OFFSET), EMAIL_SIZE);
}

void* cursor_value(Cursor* cursor)
{
	uint32_t page_num = cursor->page_num;
	void* page = get_page(cursor->table->pager, page_num);
	return leaf_node_value(page, cursor->cell_num);
}

Table* db_open(const char* filename)
{
	Pager* pager = pager_open(filename);

	Table* table = malloc(sizeof(Table));
	table->pager = pager;
	table->root_page_num = 0;

	if (pager->num_pages == 0)
	{
		// This is a new database file
		void* root_node = get_page(pager, 0);
		initialize_leaf_node(root_node);
		set_node_root(root_node, 1);
	}

	return table;
}

Pager* pager_open(const char* filename)
{
	int fd = open(filename, O_RDWR | O_CREAT, _S_IWRITE | _S_IREAD);

	if (fd == -1)
	{
		printf("Unable to open file\n");
		exit(EXIT_FAILURE);
	}

	off_t file_lenght = lseek(fd, 0, SEEK_END);

	Pager* pager = malloc(sizeof(Pager));
	pager->file_descriptor = fd;
	pager->file_lenght = file_lenght;
	pager->num_pages = (file_lenght / PAGE_SIZE);

	if (file_lenght % PAGE_SIZE != 0)
	{
		printf("Db file is not a whole of number of pages. Corrupt file.\n");
		exit(EXIT_FAILURE);
	}


	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
	{
		pager->pages[i] = 0;
	}

	return pager;
}

void* get_page(Pager* pager, uint32_t page_num)
{
	if (page_num > TABLE_MAX_PAGES)
	{
		printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
		exit(EXIT_FAILURE);
	}

	if (pager->pages[page_num] == NULL)
	{
		void* page = malloc(PAGE_SIZE);
		uint32_t num_pages = pager->file_lenght / PAGE_SIZE;

		if (pager->file_lenght % PAGE_SIZE)
			num_pages += 1;

		if (page_num <= num_pages)
		{
			lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
			SSIZE_T bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
			if (bytes_read == -1)
			{
				printf("Error reading file: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}

		pager->pages[page_num] = page;
		
		if (page_num >= pager->num_pages)
			pager->num_pages = page_num + 1;
	}

	return pager->pages[page_num];
}

void db_close(Table* table)
{
	Pager* pager = table->pager;

	for (uint32_t i = 0; i < pager->num_pages; i++)
	{
		if (pager->pages[i] == NULL)
			continue;

		pager_flush(pager, i);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	int result = close(pager->file_descriptor);
	if (result == -1)
	{
		printf("Error closing db file. \n");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
	{
		void* page = pager->pages[i];
		if (page)
		{
			free(page);
			pager->pages[i] = NULL;
		}
	}
	
	free(pager);
	free(table);
}

void pager_flush(Pager* pager, uint32_t page_num)
{
	if (pager->pages[page_num] == NULL)
	{
		printf("Tried to flush null page\n");
		exit(EXIT_FAILURE);
	}

	off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
	
	if (offset == -1)
	{
		printf("Error seeking: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	SSIZE_T bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

	if (bytes_written == -1)
	{
		printf("Error writing: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

Cursor* table_start(Table* table)
{
	Cursor* cursor = malloc(sizeof(Cursor));
	cursor->table = table;
	
	cursor->page_num = table->root_page_num;
	cursor->cell_num = 0;

	void* root_node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(root_node);
	cursor->end_of_table = (num_cells == 0);

	return cursor;
}

Cursor* table_find(Table* table, uint32_t key)
{
	uint32_t root_page_num = table->root_page_num;
	void* root_node = get_page(table->pager, root_page_num);

	if (get_node_type(root_node) == NODE_LEAF)
	{
		return leaf_node_find(table, root_page_num, key);
	}
	else
	{
		printf("Need to implement searching an internal node\n");
		exit(EXIT_FAILURE);
	}
}

void cursor_advance(Cursor* cursor)
{
	uint32_t page_num = cursor->page_num;
	void* node = get_page(cursor->table->pager, page_num);

	cursor->cell_num += 1;
	if (cursor->cell_num >= (*leaf_node_num_cells(node)))
		cursor->end_of_table = 1;
}

uint32_t get_unused_page_num(Pager* pager)
{
	return pager->num_pages;
}