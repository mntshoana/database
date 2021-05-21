#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char* buffer;
    size_t length;
    ssize_t inputLength;
}inputBuffer;

typedef struct {
    int id;
    char** col;
    int colCount; // number of coloumns (excl id)
}Row;

#define TABLE_MAX_PAGES 100
typedef struct {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
}Table;

Table* newTable();
void freeTable(Table* table);

inputBuffer* initInputBuffer();
void freeBuffer(inputBuffer* in);

void serializeRow(Row* source, void* target);
void deserializeRow(void* source, Row* target);

void* indexRow(Table* table, uint32_t row);

Row* prepareRow(int cols);
void freeRow(Row* row);

int insertRowToTable(inputBuffer* line, Table* table);
int selectfromTable(inputBuffer* line, Table* table);

void inputFromUser(inputBuffer* in);
void outputToUser();

bool isCommand(inputBuffer* in);
int processCommand(inputBuffer* cmd, Table* table);

int processStatement(inputBuffer* stmt, Table* table);
void execute(int stmt, inputBuffer* line, Table* table);
#endif
