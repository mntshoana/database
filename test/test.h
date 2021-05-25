#ifndef TEST_HEADER_FILE
#define TEST_HEADER_FILE

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int replaceStream();
void restoreStream(int backup);
char** loadFromFile();
char** createBuf(int size);
char** appendBuf(char*** buff, int idx, char* str);
void freeBuff(char** ptr);
void show(char** expected, char** result);
#endif
