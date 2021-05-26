#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

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
    int fileDescriptor;
    uint32_t fileLength;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    uint32_t num_rows;
    Pager* pager;
}Table;

Pager* initPager(const char* file);
void* getPage(Pager* pager, uint32_t pgNr);

Table* openDB(const char* file);
void updateDisk(Pager* pager, uint32_t pgNr, uint32_t size);
void closeDB(Table* table);

inputBuffer* initInputBuffer();
void freeBuffer(inputBuffer* in);

void serializeRow(Row* source, void* target);
void deserializeRow(void* source, Row* target);

void* indexRow(Table* table, uint32_t row);

Row* prepareRow(int cols);
void freeRow(Row* row);

int insertRowToTable(inputBuffer* line, Table* table);
int selectfromTable(inputBuffer* line, Table* table);
int createTable(inputBuffer* line, Table** table);

int inputFromUser(inputBuffer* in);
void outputToUser();

bool isCommand(inputBuffer* in);
int processCommand(inputBuffer* cmd, Table** tablePtr);

int processStatement(inputBuffer* stmt, Table** tablePtr);
void execute(int stmt, inputBuffer* line, Table** table);
#endif
