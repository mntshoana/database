#include "mySQLDB.h"

int main (int argc, char* argv[]){
    // The I/O loop otherwise known as the Read, Execute, Print, Loop (REPL)
    inputBuffer* in = initInputBuffer();
    while(true){
        outputToUser();
        inputFromUser(in);
        
        if (strcmp(in->buffer, ".exit") == 0){
            // Exit
            freeBuffer(in);
            exit(EXIT_SUCCESS);
        }
        else {
            printf("Error: Command not recognized - \"%s\".\n",
                   in->buffer);
        }
    }
    return 0;
}
