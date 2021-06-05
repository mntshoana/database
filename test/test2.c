describe ("Storage"){

// TEST7 "ERROR CHECK: ID MUST NOT BE NEGATIVE"
    it(TEST7){
        char* script[] = {
            "insert 1 user email@address.com",
            "insert 1 user email@address.com",
            "select",
            ".exit",
            NULL
        };
        char** result = run(TEST7, script, true);

        char* expected[] = {
                        "myDB > Inserting...",
                        "Successfully executed insert statement.",
                        "myDB > Inserting...",
                        "Error! Cannot insert duplicate keys",
                        "myDB > Selecting...",
                        "1 user email@address.com",
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
