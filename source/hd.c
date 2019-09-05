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
    free(input_buffer->buffer); // очистка выделенного буфера
    free(input_buffer); // очистка структуры
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer)
{
    if(strcmp(input_buffer->buffer, ".exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }

    if(strcmp(input_buffer->buffer, ".clear") == 0 || strcmp(input_buffer->buffer,".cls") == 0)
    {
        return clear_screen();
    }

    if(strncmp(input_buffer->buffer, ".help", 5) == 0)
    {
        return help(input_buffer);
    }

    else return META_COMMAND_UNRECOGNIZED_COMMAND; // возращаем что такой мета команды нет
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    if(strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        statement->type = STATEMENT_INSERT; // состояние INSERT
        int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), 
            statement->row_to_insert.username, statement->row_to_insert.email); // считывание 3 параметров 

        if(args_assigned < 3) // если считанных параметров меньше 3
        {
            return PREPARE_SYNTAX_ERROR; // возращаем синтаксическую ошибку
        }

        return PREPARE_SUCCESS; 
    }

    if(strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT; // возращаем что такого оператора нет
}

ExecuteResult execute_statement(Statement* statement, Table* table) 
{
    switch (statement->type) 
    {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table); // выполнение оператора INSERT
    case (STATEMENT_SELECT):
        return execute_select(statement, table); // выполнение оператора SELECT
    }
}

void serialize_row(Row* source, void* destination) 
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE); // запись ид
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE); // Переход на (ID_OFFSET + ID_SIZE) байт и запись имени
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE); // Переход на (USERNAME_OFFSET + USERNAME_SIZE) байт и запись емайла 
}

void deserialize_row(void* source, Row* destination) 
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE); // Переход на ID_OFFSET байт и считывания ID_SIZE байта 
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE); // Переход на USERNAME_OFFSET байт и считывания USERNAME_SIZE байта 
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE); // Переход на EMAIL_OFFSET байт и считывания EMAIL_SIZE байта 
}

void* row_slot(Table* table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE; // выбор страницы 
    void* page = table->pages[page_num]; 
    if(page == NULL)
    {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE; // смещение строки
    uint32_t byte_offset = row_offset * ROW_SIZE; // смещение к новой строке
    return page + byte_offset;
}

ExecuteResult execute_insert(Statement* statment, Table* table)
{
    if(table->num_rows >= TABLE_MAX_ROWS) // проверка на заполненность таблицы
    {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statment->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows)); // параметры - это сама таблица и смещение 
    table->num_rows += 1; 

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table)
{
    Row row;
    for(uint32_t i = 0; i < table->num_rows; i++)
    {
        deserialize_row(row_slot(table, i), &row); // считывание строк из памяти 
        print_row(&row); // печать строки 
    }

    return EXECUTE_SUCCESS;
}


Table* new_table() 
{
    Table* table = malloc(sizeof(Table)); 
    table->num_rows = 0;
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        table->pages[i] = NULL;
    }
    
    return table;
}

void free_table(Table* table) 
{
    for(int i = 0; table->pages[i]; i++) 
    {
        free(table->pages[i]);
    }

    free(table);
}


PrepareResult prepare_isnert(InputBuffer* input_buffer, Statement* statement)
{
    char* keyword = strtok(input_buffer->buffer, " "); // разделяем строку на подстроки (оператор)
    char* id_string = strtok(NULL, " "); // разделяем строку на подстроки (номер записаи)
    char* username = strtok(NULL, " "); // разделяем строку на подстроки (имя пользователя)
    char* email = strtok(NULL, " "); // разделяем строку на подстроки (почта пользователя)

    if(id_string == NULL || username == NULL || email == NULL) // проверка на пустоту данных
    return PREPARE_SYNTAX_ERROR;

    int id = atoi(id_string);

    if(id < 0)
        return PREPARE_NEGATIVE_ID;

    if(strlen(username) > COLUMN_USERNAME_SIZE) // проверка на размер введенного ника 
        return PREPARE_STRING_TOO_LONG;

    if(strlen(email) > COLUMN_EMAIL_SIZE) // проверка на размер введенного емайла
        return PREPARE_STRING_TOO_LONG;

    statement->row_to_insert.id = id; 
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

void print_row(Row* row)
{
    printf("(%d %s %s)\n", row->id, row->username, row->email);
}

MetaCommandResult clear_screen()
{
    system("cls");
    return META_COMMAND_SUCCESS;
}

MetaCommandResult help(InputBuffer* input_buffer)
{
    char buf[8];

    int args_assigned = sscanf(input_buffer->buffer, ".help %s", buf);

    if(args_assigned > 1 || args_assigned == -1)  
        return META_COMMAND_HELP_WRONG;

    if(strlen(buf) > 8)
        return META_COMMAND_HELP_LONG;
    
    if(strcmp(buf, "meta") == 0)
    {
        printf( "\n\t\".cls\" or \"clear\" - to clear screen of console\n"
                "\t\".exit\" - exit the program\n\n"
              );

        return META_COMMAND_SUCCESS;
    }

    if(strcmp(buf, "oper") == 0)
    {
        printf( "\n\t\"insert\" - insert a new row. Expamle: insert 1 Bastard alex@jpeg\n"
                "\t\"select\" - print all the rows\n\n"
              ); 
        return META_COMMAND_SUCCESS;
    }

    return META_COMMAND_HELP_WRONG;
}