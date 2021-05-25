#include "mySQLDB.h"

#define ERR_STRING_TOO_LARGE -2
#define ERR_ID_NOT_POSITIVE -3
#define FAILED -1
#define OK 1

// SQLITE architecture
// https://www.sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki

/*
 * | id | col1 | col2 |
 * | int| 32b  | 32b  |
 * thus row size ==
 */
#define COL_WIDTH 32
#define ROW_SIZE ( sizeof(int) + sizeof(char) * (COL_WIDTH+1) * 2 )

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;


Table* newTable(){
    Table* table = malloc(sizeof(Table));
    table->num_rows = 0;
    for (int i = 0; i < TABLE_MAX_PAGES; i++)
        table->pages[i] = NULL;
    
    return table;
}

void freeTable(Table* table){
    for (int i = 0; i < TABLE_MAX_PAGES && table->pages[i]; i++)
        free(table->pages[i]);
    free(table);
}


inputBuffer* initInputBuffer(){
    inputBuffer* buffer = (inputBuffer*)malloc(sizeof(inputBuffer));
    buffer->buffer = NULL;
    buffer->length = 0;
    buffer->inputLength = 0;
    
    return buffer;
}

void freeBuffer(inputBuffer* in){
    free(in->buffer);
    free(in);
    return;
}


void serializeRow(Row* source, void* target){
    memcpy(target, &(source->id), sizeof(int));
    memcpy(target + sizeof(int), &(source->col), sizeof(char) * (COL_WIDTH+1) * source->colCount);
}

void deserializeRow(void* source, Row* target){
    memcpy(&(target->id), source, sizeof(int));
    memcpy(&(target->col), source + sizeof(int), sizeof(char) * (COL_WIDTH+1) * target->colCount);
}

void* indexRow(Table* table, uint32_t row){
    uint32_t pgNr  = row / ROWS_PER_PAGE;
    void* page = table->pages[pgNr];
    if (page == NULL)
        page = table->pages[pgNr] = malloc(PAGE_SIZE);
    
    uint32_t offset = (row % ROWS_PER_PAGE) * ROW_SIZE;
    return page + offset;
}

Row* prepareRow(int cols){
    Row* temp = (Row*)malloc(sizeof(Row));
    temp->colCount = cols; // id col + 2 other columns
    
    // adjust col size of the line
    temp->col = (char**)malloc( sizeof(char*) * temp->colCount);
    // adjust space for each col
    for (int i = 0; i < temp->colCount; i++)
        temp->col[i] = (char*)malloc( sizeof(char) * (COL_WIDTH+1));
    
    return temp;
}

void freeRow(Row* row){
    for (int i = 0; i < row->colCount; i++)
        free(row->col[i]); // space for each col
    free(row->col); // cols
    free(row);
}

int insertRowToTable(inputBuffer* line, Table* table){
    Row* row = prepareRow(2); // 2 coloums required + id column
    
    char* instruction = strtok(line->buffer, " ");
    char* colId = strtok(NULL, " ");
    char* col2 = strtok(NULL, " ");
    char* col3 = strtok(NULL, " ");
    //int res = sscanf(line->buffer, "insert %d %s %s",
      //               &row->id, row->col[0], row->col[1]);
    
    if (instruction  == NULL || colId == NULL || col2 == NULL || col3 == NULL){
        free(row);
        return FAILED;
    }
    row->id = atoi(colId);
    if (row->id < 0){
        free(row);
        return ERR_ID_NOT_POSITIVE;
    }
    if (strlen(col2) > COL_WIDTH || strlen(col3) > COL_WIDTH){
        free(row);
        return ERR_STRING_TOO_LARGE;
    }
    strcpy(row->col[0], col2);
    strcpy(row->col[1], col3);
    
    //insert Row To Table
    serializeRow(row, indexRow(table, table->num_rows));
    table->num_rows++;
    
    free(row);
    return OK;
    
}

int selectfromTable(inputBuffer* line, Table* table){
    Row* row = prepareRow(2); // 2 columns requred + id coloum
    int res = sscanf(line->buffer, "select ...\n"); // Todo
    
    if (res < row->colCount + 1){
    //    free(row);
    //    return -1;
    }
    
    //retrieve Rows from Table
    for (uint32_t i = 0; i < table->num_rows; i++){
        deserializeRow(indexRow(table, i), row);
        printf("%d %s %s\n", row->id, row->col[0], row->col[1]);
    }
    
    free(row);
    return OK;
}

int inputFromUser(inputBuffer* in){
    ssize_t inBytes = getline(&in->buffer, &in->length, stdin);
    if (inBytes <= 0){ // failed to input
        return FAILED;
    }
    
    // remember the trailing newline
    in->inputLength = inBytes - 1;
    in->buffer[inBytes - 1] = '\0';
    return OK;
}

void outputToUser(){
    printf("myDB > ");
    return;
}

bool isCommand(inputBuffer* in){
    if (in->buffer[0] == '.')
        return true;
    else
        return false;
}

int processCommand(inputBuffer* cmd, Table* table){
    if (strcmp(cmd->buffer, ".exit") == 0){
        // Exit
        freeBuffer(cmd);
        freeTable(table);
        exit(EXIT_SUCCESS);
    }
    // Todo add more commands
    return FAILED;
}

#define INSERT 1
#define SELECT 2

int processStatement(inputBuffer* stmt, Table* table){
    if (strncmp(stmt->buffer, "insert", 6) == 0)
        execute(INSERT, stmt, table);
    else if (strncmp(stmt->buffer, "select", 6) == 0)
        execute(SELECT, stmt, table);
    else
        return FAILED;
    
    return OK;
}

void execute(int stmt, inputBuffer* line, Table* table){
    int res;
    switch (stmt){
        case INSERT:
            if (table->num_rows >= TABLE_MAX_ROWS){
                printf("Error! Table is already full");
                break;
            }
            printf("Inserting...");
            res = insertRowToTable(line, table);
            if (res == OK)
                printf("Successfully executed insert statement.");
            else if (res == ERR_STRING_TOO_LARGE)
                printf("Error! Length of string cannot exceed %d.", COL_WIDTH);
            else if (res == ERR_ID_NOT_POSITIVE)
                printf("Error! ID is required to be positive");
            else
                printf("Syntax Error!");
            break;
        case SELECT:
            printf("Selecting...\n");
            res = selectfromTable(line, table);
            if (res == OK)
                printf("Successfully executed select statement.");
            else
                printf("Sytax Error");
            break;
    }
    printf("\n");
}


