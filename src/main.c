#include "mySQLDB.h"

int main (int argc, char* argv[]){
    // The I/O loop otherwise known as the Read, Execute, Print, Loop (REPL)
    Table* table = newTable();
    inputBuffer* in = initInputBuffer();
    while(true){
        outputToUser();
        inputFromUser(in);
        int res;
        if (isCommand(in)) // process as command
            res processCommand(in, table);
        else  // process as SQL statement
            res = processStatement(in, table);
        
        if (res == -1)
            printf("Error: %s not recognized - \"%s\".\n", isCommand(in) ? "Command":"Keyword", in->buffer);

    }
    return 0;
}
