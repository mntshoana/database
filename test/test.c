#include "test.h"
#include "bdd-for-c.h"

void header(char* title){
    int len = strlen(title) + 18;
    char* border = (char*)malloc(sizeof(char)* len +1);
    border[len] = '\0';
    memset(border, '+', len);
    printf("\n%s", border);
    printf("\n...RUNNING: \"%s\"...\n", title);
    printf("%s\n\n", border);
    free(border);
}
char** run(char* title, char** script){
    header(title);
    int backup = replaceStream();
    char* appPath = "bin/mySQLDB.o";
    FILE* fp = popen(appPath, "w");
    for(int i = 0; ; i++){
        if (script[i] == NULL)
            break;
        
        if (script[i+1] == NULL)
            fprintf( fp, "%s\r", script[i]);
        else
            fprintf( fp, "%s\n", script[i]);
    }
    pclose(fp);
    restoreStream(backup);
    return loadFromFile();
    
}


spec ("main"){
#define TEST1 "STATEMENTS: INSERT and SELECT"
    it (TEST1 ){
        char* script[] = {  "insert 1 user email@address.com",
                            "select",
                            ".exit",
                            NULL };
        char** result = run(TEST1, script);
        
        char* expected[] = {
                        "myDB > Inserting...Successfully executed insert statement.",
                        "myDB > Selecting...",
                        "1 user email@address.com",
                        "Successfully executed select statement.",
                        "myDB >",
                        NULL
        };
        
        show(expected, result);
        // Compare all
        for (int i = 0; ; i++){
            if (result[i] == NULL || expected[i] == NULL)
                break;
            int len = strlen(result[i]);
            check(strncmp(result[i], expected[i], len) == 0);
        }
        
        freeBuff(result);
    }

#define TEST2 "ERROR CHECK: Table Full"
    it (TEST2){
        const int count = 6002;
        char** script = (char**)malloc(sizeof(char*) * (count + 1) ); // final NULL
        char* line = (char*)malloc(255);
        int i;
        for ( i = 1; i < count; i++){
            sprintf(line, "insert %d user%d person%d@example.com", i, i, i);
            appendBuf(&script, i-1, line);
        }
        appendBuf(&script, i-1, ".exit");
        char** result = run(TEST2, script);
        char* expected[] = {
                        "myDB > Error! Table is already full",
                        NULL
        };
        
        // COMPARE
        for(i = count; ; i++){
            if (result[i] == NULL)
                break;
        }
        i -= 2; // last == NULL; last -1 == "DB >" which is the exit line
        printf("Expected:\n");
        printf("%s\n", expected[0]);
        printf("\n");
        
        printf("Results:\n");
        printf("%s\n", result[i]);
        printf("\n");
        
        int len = strlen(result[0]);
        check(strncmp(result[i], expected[0], len) == 0);
        
        freeBuff(result);
        freeBuff(script);
    }
    
#define A_32 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
#define TEST3 "INSERTING STRINGS OF MAXIMUM LENGTH"
    it(TEST3){
        char* script[] = {
            "insert 1 "A_32" "A_32,
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST3, script);

        char* expected[] = {
                        "myDB > Inserting...Successfully executed insert statement.",
                        "myDB > Selecting...",
                        "1 "A_32" "A_32,
                        "Successfully executed select statement.",
                        "myDB >",
                        NULL
        };
        
        show(expected, result);
        // Compare all
        for (int i = 0; ; i++){
            if (result[i] == NULL || expected[i] == NULL)
                break;
            int len = strlen(result[i]);
            check(strncmp(result[i], expected[i], len) == 0);
        }
        
        freeBuff(result);
    }
    
#define A_33 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
#define TEST3 "ERROR CHECK: STRINGS OVER MAXIMUM LENGTH"
    it(TEST3){
        char* script[] = {
            "insert 1 "A_33" "A_33,
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST3, script);

        char* expected[] = {
                        "myDB > Inserting...Error! Length of string cannot exceed 32.",
                        "myDB > Selecting...",
                        "Successfully executed select statement.",
                        "myDB > ",
                        NULL
        };
        
        show(expected, result);
        // Compare all
        for (int i = 0; ; i++){
            if (result[i] == NULL || expected[i] == NULL)
                break;
            int len = strlen(result[i]);
            check(strncmp(result[i], expected[i], len) == 0);
        }
        
        freeBuff(result);
    }
    
#define TEST4 "ERROR CHECK: ID MUST NOT BE NEGATIVE"
    it(TEST4){
        char* script[] = {
            "insert -1 this andThat",
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST4, script);

        char* expected[] = {
                        "myDB > Inserting...Error! ID is required to be positive",
                        "myDB > Selecting...",
                        "Successfully executed select statement.",
                        "myDB > ",
                        NULL
        };
        
        show(expected, result);
        // Compare all
        for (int i = 0; ; i++){
            if (result[i] == NULL || expected[i] == NULL)
                break;
            int len = strlen(result[i]);
            check(strncmp(result[i], expected[i], len) == 0);
        }
        
        freeBuff(result);
    }
}
