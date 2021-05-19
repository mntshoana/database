// SQLITE architecture
// https://www.sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki

#include "mySQLDB.h"

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
void inputFromUser(inputBuffer* in){
    ssize_t inBytes = getline(&in->buffer, &in->length, stdin);
    if (inBytes <= 0){ // failed to input
        printf("Error: unable to read the user input\n");
        exit(EXIT_FAILURE);
    }
    
    // remember the trailing newline
    in->inputLength = inBytes - 1;
    in->buffer[inBytes - 1] = '\0';
    return;
}

void outputToUser(){
    printf("myDB > ");
    return;
}


