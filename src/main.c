#include "mySQLDB.h"

int main (int argc, char* argv[]){
    // The I/O loop otherwise known as the Read, Execute, Print, Loop (REPL)
    Table* table = newTable();
    inputBuffer* in = initInputBuffer();
    while(true){
        outputToUser();
        inputFromUser(in);
        
        if (isCommand(in)){ // process as command
            int res = processCommand(in, table);
            if (res == -1)
                printf("Error: Command not recognized - \"%s\".\n", in->buffer);
        }
        else { // process as SQL statement
            int res = processStatement(in, table);
            if (res == -1)
                printf("Error: Keyword not recognized - \"%s\".\n", in->buffer);
        }

    }
    return 0;
}
