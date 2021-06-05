#ifndef TEST_HEADER_FILE
#define TEST_HEADER_FILE

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

int replaceStream();
void restoreStream(int backup);
char** loadFromFile();
char** createBuf(int size);
char** appendBuf(char*** buff, int idx, char* str);
void freeBuff(char** ptr);
void show(char** expected, char** result);
void showLine(char** expected, int i, char** result, int j);

#define RESTART -1
void header(char* title);
char** run(char* title, char** script, bool withArgs);

#define TEST1 "STATEMENTS - INSERT and SELECT"
#define TEST2 "ERROR CHECK - Table Full"
#define TEST3 "CHECK - INSERTING STRINGS OF MAXIMUM LENGTH"
#define TEST4 "ERROR CHECK - STRINGS OVER MAXIMUM LENGTH"
#define TEST5 "ERROR CHECK - ID MUST NOT BE NEGATIVE"
#define TEST6 "CHECK - DATA PERSISTS AFTER CLOSING CONNECTION"

#define TEST7 "ERROR CHECK - DUPLICATE KEYS"
#endif
