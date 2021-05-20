// SQLITE architecture
// https://www.sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki

#include "mySQLDB.h"

table* db;

inputBuffer* initInputBuffer(){
    inputBuffer* buffer = (inputBuffer*)malloc(sizeof(inputBuffer));
    buffer->buffer = NULL;
    buffer->length = 0;
    buffer->inputLength = 0;
    
    return buffer;
}

void freeBuffer(inputBuffer* in){
    free(in->buffer);
    free(in);
    return;
}

void initDatabase(){
    db = (table*)malloc(sizeof(table));
    db->size = 0;
    db->cols = 2; // id col + 2 other columns
}

int insertToDatabase(inputBuffer* line){
    if (db == NULL)
        initDatabase();
    // Add line
    db->id = (int*)realloc(db->id, sizeof(int) * (db->size +1) );
    db->row = (char***)realloc(db->row, sizeof(char**) * (db->size +1) );
    
    // adjust col size of the line
    db->row[db->size] = (char**)realloc(db->row[db->size], sizeof(char*) * db->cols);
    // adjust space for each col
    for (int i = 0; i < db->cols; i++)
        db->row[db->size][i] = (char*)realloc(db->row[db->size][i], sizeof(char) * 32);
    
    int res = sscanf(line->buffer, "insert %d %s %s",
                     &db->id[db->size], db->row[db->size][0], db->row[db->size][1]);
    
    if (res < db->cols){
        return -1;
    }
    db->size++;
    return 1;
    
}
void freeDatabase(){
    if (db == NULL)
        return;

    for (int i = 0; i < db->size; i++){
        for (int j = 0; j < db->cols; j++)
            free(db->row[i][j]); // space for each col
        free(db->row[i]); // cols
    }
    free(db->row);//rows
    free(db->id);
    
    free (db);
    db = NULL;
}

void inputFromUser(inputBuffer* in){
    ssize_t inBytes = getline(&in->buffer, &in->length, stdin);
    if (inBytes <= 0){ // failed to input
        printf("Error: unable to read the user input\n");
        exit(EXIT_FAILURE);
    }
    
    // remember the trailing newline
    in->inputLength = inBytes - 1;
    in->buffer[inBytes - 1] = '\0';
    return;
}

void outputToUser(){
    printf("myDB > ");
    return;
}

bool isCommand(inputBuffer* in){
    if (in->buffer[0] == '.')
        return true;
    else
        return false;
}

int processCommand(inputBuffer* cmd){
    if (strcmp(cmd->buffer, ".exit") == 0){
        // Exit
        freeBuffer(cmd);
        freeDatabase();
        exit(EXIT_SUCCESS);
    }
    // Todo add more commands
    return -1;
}

#define INSERT 1
#define SELECT 2

int processStatement(inputBuffer* stmt){
    if (strncmp(stmt->buffer, "insert", 6) == 0)
        execute(INSERT, stmt);
    else if (strncmp(stmt->buffer, "select", 6) == 0)
        execute(SELECT, stmt);
    else
        return -1;
    
    return 1;
}

void execute(int stmt, inputBuffer* line){
    switch (stmt){
        case INSERT:
            printf("Inserting...");
            int res = insertToDatabase(line);
            if (res == 1)
                printf("Successfully executed insert statement.");
            else
                printf("Syntax Error!");
            break;
        case SELECT:
            printf("Selecting...");
            // Todo
            printf("Successfully executed select statement.");
            break;
    }
    printf("\n");
}


