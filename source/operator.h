#pragma once
#include <stdint.h>
#include "input.h"

#define COLUMN_USERNAME_SIZE 10
#define COLUMN_EMAIL_SIZE 25

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define PAGE_SIZE 4096
#define TABLE_MAX_PAGES 100



typedef enum 
{
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT, 
    PREPARE_SYNTAX_ERROR,
	PREPARE_NEGATIVE_ID,
	PREPARE_STRING_TOO_LONG
} PrepareResult;

typedef enum 
{
    STATEMENT_INSERT, 
    STATEMENT_SELECT
} StatementType;

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct 
{
    StatementType type;
    Row row_to_insert;
} Statement;

typedef enum 
{
    EXECUTE_SUCCCESS,
	EXECUTE_DUPLICATE_KEY,
    EXECUTE_TABLE_FULL
} ExecuteResult;



typedef struct {
	int file_descriptor;
	uint32_t file_lenght;
	uint32_t num_pages;
	void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
	Pager* pager;
	uint32_t root_page_num;
} Table;

typedef struct 
{
	Table* table;
	uint32_t page_num;
	uint32_t cell_num;
	_Bool end_of_table;
} Cursor;

typedef enum 
{
	NODE_INTERNAL, 
	NODE_LEAF 
} NodeType;



PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statment);
ExecuteResult execute_statement(Statement* statement, Table* table);
ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
void* cursor_value(Cursor* cursor);
void print_row(Row* row);
Table* db_open(const char* filename);
Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void db_close(Table* table);
void pager_flush(Pager* pager, uint32_t page_num);
Cursor* table_start(Table* table);
Cursor* table_find(Table* table, uint32_t key);
void cursor_advance(Cursor* cursor);
uint32_t get_unused_page_num(Pager* pager);