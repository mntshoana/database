#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


// SQLITE architecture
// https://www.sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki

/*
 * | id | col1 | col2 |
 * | int| 32b  | 32b  |
 * thus row size ==
 */
#define COL_WIDTH 32
#define ROW_SIZE ( sizeof(int) + sizeof(char) * (COL_WIDTH+1) * 2 )

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

#define PAGE_SIZE 4096
#define TABLE_MAX_PAGES 100
typedef struct {
    int fileDescriptor;
    uint32_t fileLength;
    uint32_t pageCount;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    uint32_t rootPage;
    Pager* pager;
}Table;

typedef struct {
    Table* table;
    uint32_t pgNr;
    uint32_t cellNr;
    bool endOfTable;
} TableCursor;

Pager* initPager(const char* file);
void* getPage(Pager* pager, uint32_t pgNr);

Table* openDB(const char* file);
void updateDisk(Pager* pager, uint32_t pgNr);
void closeDB(Table* table);

TableCursor* findFromTable(Table* table, uint32_t key);
TableCursor* tableStart(Table* table);
TableCursor* tableEnd(Table* table);

void next(TableCursor* cursor);

inputBuffer* initInputBuffer();
void freeBuffer(inputBuffer* in);

void serializeRow(Row* source, void* target);
void deserializeRow(void* source, Row* target);

void* indexValue(TableCursor* cursor);

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
