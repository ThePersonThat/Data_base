#ifndef _HD_H_
    #define _HG_H

#include <sys\types.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
 

#define ID_SIZE (size_of_attribute(Row, id))
#define USERNAME_SIZE (size_of_attribute(Row, username))
#define EMAIL_SIZE (size_of_attribute(Row, email)) 
#define ID_OFFSET 0
#define USERNAME_OFFSET (ID_OFFSET + ID_SIZE)
#define EMAIL_OFFSET (USERNAME_OFFSET + USERNAME_SIZE)
#define ROW_SIZE (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)

#define PAGE_SIZE 4096
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)
#define TABLE_MAX_ROWS (ROWS_PER_PAGE * TABLE_MAX_PAGES)

typedef enum
{
    EXECUTE_SUCCESS, 
    EXECUTE_TABLE_FULL 
}ExecuteResult;

typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
}MetaCommandResult; // перечисление для результатов мета команд, таких как ".exit"

typedef enum
{
    PREPARE_SUCCESS, 
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT // перечисление для результатов мета команд 
}PrepareResult;

typedef enum 
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
}StatementType; // перечисление для списка операторов

typedef struct 
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row; // пользовательская строка


typedef struct 
{
    StatementType type;
    Row row_to_insert;  // только для оператора INSERT
}Statement;

typedef struct 
{
    char* buffer; // указатель на буфер, который будет исользоваться для текста
    size_t buffer_length; // константный размер буфера
    ssize_t input_length; // размер буфера который ввел пользователь 
}InputBuffer;


typedef struct 
{
    uint32_t num_rows; // последняя заполненная строка 
    void* pages[TABLE_MAX_PAGES]; // кол-во страниц по 4096 байт  
}Table;

InputBuffer *new_input_buffer(); // инизиализация буфера
void print_prompt(); // инвайт на ввод пользователя
void read_input(InputBuffer* input_buffer); // чтение
void close_input_buffer(InputBuffer* input_buffer); // очистка памяти
MetaCommandResult do_meta_command(InputBuffer* input_buffer); // функция для обработки мета команд
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement); // функция для обработки операторов
ExecuteResult execute_statement(Statement* statement, Table* table); // функция для выполнения операторов
ExecuteResult execute_insert(Statement* statment, Table* table); // выполнение оператора insert
ExecuteResult execute_select(Statement* statement, Table* table); // выполнение оператора select
void serialize_row(Row* source, void* destination); // запись 
void deserialize_row(void* source, Row* destination);
void* row_slot(Table* table, uint32_t row_num); // поиск смещения 
Table* new_table(); // создание таблицы
void free_table(Table* table); // очистка памяти
void print_row(Row* row); // печать строки
#endif