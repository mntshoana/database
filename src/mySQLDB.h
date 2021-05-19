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

inputBuffer* initInputBuffer();
void freeBuffer(inputBuffer* in);

void inputFromUser(inputBuffer* in);
void outputToUser();
#endif
