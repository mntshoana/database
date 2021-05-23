#include <fcntl.h>

#include "bdd-for-c.h"
#include "../src/mySQLDB.h"

int changedStream;


fpos_t pos;
int replaceStream(){
    system("echo '' > bin/write.txt"); //  file exists, is empty
    changedStream = open("bin/write.txt", O_WRONLY); // open a file
    
    int backup = dup(STDOUT_FILENO); // make backup of stdout
    
    
    fflush(stdout); //clean everything first
    
    dup2(changedStream, STDOUT_FILENO ); // redirect printf() into file
    return backup;
}
void restoreStream(int backup){
    fflush(stdout); //clean everything first
    
    dup2(backup, STDOUT_FILENO);
    
    close(changedStream);
    fsetpos(stdout, &pos);
}


void run(char* script[]){
    Table* table = newTable();
    inputBuffer* instruction = initInputBuffer();
    instruction->buffer = (char*)malloc(255);
    
    for (int i = 0 ; ; i++){
        if (script[i] == NULL)
            break;
        strcpy(instruction->buffer, script[i]);
        instruction->inputLength = strlen(script[i]);
        instruction->buffer[instruction->inputLength] = '\0';
        int res;
        if (isCommand(instruction)) // process as command
            res = processCommand(instruction, table);
        else  // process as SQL statement
            res = processStatement(instruction, table);
        
        if (res == -1)
            printf("Error: %s not recognized - \"%s\".\n", isCommand(instruction) ? "Command":"Keyword", instruction->buffer);

    }
    freeBuffer(instruction);
    freeTable(table);
}

char* loadResults(){
    FILE *file;
    file = fopen("bin/write.txt", "r");

    fseek( file , 0L , SEEK_END);
    long lSize = ftell( file );
    rewind( file );
    
    char* str = (char*)malloc(lSize + 1);
    
    fread(str, lSize, 1, file);
    
    fclose(file);
    
    return str;
}

spec ("main"){
    
    //TEST 1
    it ("STATEMENTS: INSERT and SELECT" ){
        // PREPARE
        
        
        char* script[] = {
                    "insert 1 user email@address.com",
                    "select",
                    
                    NULL
        };
        // BEGIN
        int backup = replaceStream();
        run(script);
        restoreStream(backup);
    
        
        //RESULT
        char* result = loadResults();
        char* expected[] = {
                        "Inserting...Successfully executed insert statement.",
                        "Selecting...",
                        "1 user email@address.com",
                        "Successfully executed select statement.",
                        "myDB > ",
                        NULL
        };
        printf("Expected:\n");
        for (int i = 0; ; i++){
            if (expected[i] == NULL)
                break;
            printf("%s\n", expected[i]);
        }
        printf("Result: \n%s\n", result);
        //check(strcmp(result, expected) == 0);
        free (result);
    }

//    //TEST 2
//    it ("ERROR CHECK: Table Full"){
//        script = (1..3000).map do |i|
//            "insert #{i} user#{i} person#{i}@example.com"
//        end
//        script << ".exit"
//        expected = execute(script)
//        expect(expected[-2]).to eq('myDB > Error! Table is already full')
//    end
}
