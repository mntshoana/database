#include "mySQLDB.h"

int main (int argc, char* argv[]){
    // The I/O loop otherwise known as the Read, Execute, Print, Loop (REPL)
    Table* table = NULL;
    if (argc >= 2)
        table = openDB(argv[1]);
    
    inputBuffer* in = initInputBuffer();
    while(true){
        outputToUser();
        int res = inputFromUser(in);
        if (res == -1){
            printf("Error: unable to read the user input\n");
            freeBuffer(in);
            closeDB(table);
            exit(EXIT_FAILURE);
        }
        
        if (isCommand(in)) // process as command
            res = processCommand(in, &table);
        else  // process as SQL statement
            res = processStatement(in, &table);
        
        if (res == -1)
            printf("Error: %s not recognized - \"%s\".\n", isCommand(in) ? "Command":"Keyword", in->buffer);

    }
    return 0;
}
