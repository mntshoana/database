// SQLITE architecture
// https://www.sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
typedef struct {
    char* buffer;
    size_t length;
    ssize_t inputLength;
}inputBuffer;

inputBuffer* initInputBuffer(){
    inputBuffer* buffer = (inputBuffer*)malloc(sizeof(inputBuffer));
    buffer->buffer = NULL;
    buffer->length = 0;
    buffer->inputLength = 0;
    
    return buffer;
}
int main (int argc, char* argv[]){
    // The Read, Execute, Print, Loop (REPL)
    inputBuffer* in = initInputBuffer();
    while(true){
        printf("myDB > ");
        
        ssize_t inBytes = getline(&in->buffer, &in->length, stdin);
        if (inBytes <= 0){ // failed to input
            printf("Error: unable to read the user input\n");
            exit(EXIT_FAILURE);
        }
        
        // Ignore trailing newline
        in->inputLength = inBytes - 1;
        in->buffer[inBytes - 1] = '\0';
        
        if (strcmp(in->buffer, ".exit") == 0){
            // Exit
            free(in->buffer);
            free(in);
            exit(EXIT_SUCCESS);
        }
        else {
            printf("Error: Command not recognized - \"%s\".\n",
                   in->buffer);
        }
    }
    return 0;
}
