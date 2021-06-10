describe ("Basic"){

// TEST1 "STATEMENTS: INSERT and SELECT"
    it (TEST1 ){
        char* script[] = {  "insert 1 user email@address.com",
                            "select",
                            ".exit",
                            NULL };
        char** result = run(TEST1, script, true);
        
        char* expected[] = {
                        "myDB > Inserting...",
                        "Successfully executed insert statement.",
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
/*
// TEST2 "ERROR CHECK: Table Full"
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
        char** result = run(TEST2, script, true);
        char* expected[] = {
                        "Error! Table is already full",
                        NULL
        };
        
        // COMPARE
        for(i = count; ; i++){
            if (result[i] == NULL)
                break;
        }
        i -= 2; // last == NULL; last -1 == "DB >" which is the exit line
        showLine(expected, 0, result, i);
        
        int len = strlen(result[0]);
        check(strncmp(result[i], expected[0], len) == 0);
        
        freeBuff(result);
        freeBuff(script);
    }
*/
#define A_32 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
// TEST3 "INSERTING STRINGS OF MAXIMUM LENGTH"
    it(TEST3){
        char* script[] = {
            "insert 1 "A_32" "A_32,
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST3, script, true);

        char* expected[] = {
                        "myDB > Inserting...",
                        "Successfully executed insert statement.",
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
// TEST4 "ERROR CHECK: STRINGS OVER MAXIMUM LENGTH"
    it(TEST4){
        char* script[] = {
            "insert 1 "A_33" "A_33,
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST4, script, true);

        char* expected[] = {
                        "myDB > Inserting...",
                        "Error! Length of string cannot exceed 32.",
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
    
// TEST5 "ERROR CHECK: ID MUST NOT BE NEGATIVE"
    it(TEST5){
        char* script[] = {
            "insert -1 this andThat",
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST5, script, true);

        char* expected[] = {
                        "myDB > Inserting...",
                        "Error! ID is required to be positive",
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
    
// TEST6 "CHECK: DATA PERSISTS AFTER CLOSING CONNECTION"
    it(TEST6){
        remove("bin/persist.db");
        
        char* script[] = {
            "create table bin/persist.db",
            "insert 1 this andThat",
            ".exit",
            RESTART,
            ".open bin/persist.db",
            "select",
            ".exit",
            NULL
        };
        
        char** result1 = run(TEST6, script, false);

        char* expected1[] = {
                        "myDB > Creating...",
                        "Successfully created table",
                        "myDB > Inserting...",
                        "Successfully executed insert statement.",
                        "myDB > myDB > Openning...",
                        "myDB > Selecting...",
                        "1 this andThat",
                        "Successfully executed select statement.",
                        "myDB > ",
                        NULL
        };
        
        show(expected1, result1);
        // Compare all
        for (int i = 0; ; i++){
            if (result1[i] == NULL || expected1[i] == NULL){
                if (result1[i] != NULL || expected1[i] != NULL)
                    check(false, "Results and Expected arrays are not the same size");
                break;
            }
            int len = strlen(result1[i]);
            check(strncmp(result1[i], expected1[i], len) == 0);
        }
        
        freeBuff(result1);
    }

}

