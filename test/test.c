#include <string.h>
#include <fcntl.h>

#include "bdd-for-c.h"
#include "../src/mySQLDB.h"

int changedStream;

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

char** loadResult(){
    FILE *file;
    file = fopen("bin/write.txt", "r");
    
    size_t sLine = 1024;
    char* line = (char*)malloc(sLine);
    
    char** result = NULL;
    for (int i = 0; ; i++){
        int len = getline(&line, &sLine, file);
        result = (char**)realloc(result, sizeof(char*) * (i + 1) );
        if (len < 0){
            result[i] = NULL;
            break;
        }
    
        result[i] = (char*)malloc(len+1);
        strncpy(result[i], line, len-1);
        result[i][len-1] = '\0';
    
    }
    
    fclose(file);
    free(line);
    
    return result;
}

char** append(char*** buff, int idx, char* str, int resize){
    int len = strlen(str);
    
    if (resize == 1)
        *buff = (char**)realloc(*buff, sizeof(char*) * (idx + 1 + 1) ); // final NULL to signal end of array
    char** buffer = *buff;
    buffer[idx] = (char*)realloc(buffer[idx], len+1);
    strncpy(buffer[idx], str, len);
    buffer[idx][len] = '\0';

    buffer[idx+1] = NULL;
    
    return buffer;
}

void cleanUp(char** ptr){
    if (ptr == NULL)
        return;
    for (int i = 0; ptr[i] != NULL; i++){
        free(ptr[i]);
    }
    free(ptr);
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
        
        // EXECUTE
        int backup = replaceStream();
        run(script);
        restoreStream(backup);
        
        // PREPARE RESULT
        char** result = loadResult();
        char* expected[] = {
                        "Inserting...Successfully executed insert statement.",
                        "Selecting...",
                        "1 user email@address.com",
                        "Successfully executed select statement.",
                        NULL
        };
        
        // COMPARE
        printf("Expected:\n");
        for (int i = 0; ; i++){
            if (expected[i] == NULL)
                break;
            printf("%s\n", expected[i]);
        }
        printf("\n");
        
        printf("Results:\n");
        for (int i = 0; ; i++){
            if (result[i] == NULL)
                break;
            printf("%s\n", result[i]);
        }
        printf("\n");
        
        for (int i = 0; ; i++){
            if (result[i] == NULL || expected[i] == NULL)
                break;
            int len = strlen(result[i]);
            check(strncmp(result[i], expected[i], len) == 0);
            
        }
        cleanUp(result);
    }

    //TEST 2
    it ("ERROR CHECK: Table Full"){
        // PREPARE
        char* intr = (char*)malloc(255);
        char** script = NULL;
        int i;
        const int count = 6001;
        
        script = (char**)malloc(sizeof(char*) * (count + 1) ); // final NULL
        for ( i = 1; i <= count; i++){
            sprintf(intr, "insert %d user%d person%d@example.com", i, i, i);
            append(&script, i-1, intr, 0);
        }
        
        // EXECUTE
        int backup = replaceStream();
        run(script);
        restoreStream(backup);

        // PREPARE RESULT
        char** result = loadResult();
        char* expected[] = {
                        "Error! Table is already full",
                        NULL
        };
        
        // COMPARE
        for(i = count; ; i++){
            if (result[i] == NULL)
                break;
        }
        i -= 1;
        printf("Expected:\n");
        printf("%s\n", expected[0]);
        printf("\n");
        
        printf("Results:\n");
        printf("%s\n", result[i]);
        printf("\n");
        
        int len = strlen(result[0]);
        check(strncmp(result[i], expected[0], len) == 0);
        
        cleanUp(result);
        cleanUp(script);
    }
    
}
