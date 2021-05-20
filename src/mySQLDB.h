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
    int* id;
    char*** row;
    int size;
    int cols;
}table;

inputBuffer* initInputBuffer();
void freeBuffer(inputBuffer* in);

void initDatabase();
int insertToDatatabase();
void freeDatabase();

void inputFromUser(inputBuffer* in);
void outputToUser();

bool isCommand(inputBuffer* in);
int processCommand(inputBuffer* cmd);

int processStatement(inputBuffer* stmt);
void execute(int stmt, inputBuffer* line);
#endif
