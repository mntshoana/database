#include "mySQLDB.h"

#define ERR_STRING_TOO_LARGE -2
#define ERR_ID_NOT_POSITIVE -3
#define FAILED -1
#define OK 1

// SQLITE architecture
// https://www.sqlite.org/zipvfs/doc/trunk/www/howitworks.wiki

/*
 * | id | col1 | col2 |
 * | int| 32b  | 32b  |
 * thus row size ==
 */
#define COL_WIDTH 32
#define ROW_SIZE ( sizeof(int) + sizeof(char) * (COL_WIDTH+1) * 2 )

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

Pager* initPager(const char* file){
    // Initialize pager
    int fd = open(file, O_RDWR /*read,write*/
                        | O_CREAT, /*Create if not found*/
                  S_IWUSR | S_IRUSR); // User read+write permission
    if (fd == -1){
        printf("Error: Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    
    off_t len = lseek(fd, 0, SEEK_END);
    Pager* pager = (Pager*)malloc(sizeof(Pager));
    pager->fileDescriptor = fd;
    pager->fileLength = len;
    
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }
    return pager;
}

void* getPage(Pager* pager, uint32_t pgNr){
    if (pgNr > TABLE_MAX_PAGES){
        printf("Error: Unable to get page - page %d out of bounds (Max = %d).", pgNr, TABLE_MAX_PAGES );
        exit(EXIT_FAILURE);
    }
    
    // Check cache
    if (pager->pages[pgNr] == NULL){
        // Empty
        void* page = malloc(PAGE_SIZE);
        uint32_t pageCount = pager->fileLength / PAGE_SIZE;
        if (pager->fileLength % PAGE_SIZE)
            pageCount++; //  to NOT chop off end of page
        
        if (pgNr <= pageCount) {
            lseek(pager->fileDescriptor, pageCount * PAGE_SIZE, SEEK_SET);
            ssize_t bytes = read(pager->fileDescriptor, page, PAGE_SIZE);
            if (bytes == -1){
                printf("Error: Couldn't read file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        
        pager->pages[pgNr] = page;
    }
    
    return pager->pages[pgNr];
}

Table* openDB(const char* file){
    // Initialize table
    Table* table = malloc(sizeof(Table));
    table->pager = initPager(file);
    table->num_rows = table->pager->fileLength / ROW_SIZE;
    
    return table;
}

void updateDisk(Pager* pager, uint32_t pgNr, uint32_t size){
    if (pager->pages[pgNr] == NULL){
        printf("Error: Attempting to store a null page.\n");
        exit(EXIT_FAILURE);
    }
    
    off_t pos = lseek( pager->fileDescriptor, pgNr * PAGE_SIZE, SEEK_SET);
    if (pos == -1){
        printf("Error: Unable to store page - failure to seek.\n");
        exit(EXIT_FAILURE);
    }
    
    ssize_t bytes  = write(pager->fileDescriptor, pager->pages[pgNr], size);
    
    if (bytes == -1){
        printf("Error: Failed to write to disk - %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void closeDB(Table* table){
    Pager* pager = table->pager;
    size_t count = table->num_rows / ROWS_PER_PAGE;
    for (size_t i = 0; i < count; i++){
        if (pager->pages[i] == NULL)
            continue;
        
        updateDisk(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }
    
    uint32_t remainder = table->num_rows % ROWS_PER_PAGE;
    if (remainder > 0){
        if (pager->pages[count] != NULL){
            updateDisk(pager, count, remainder * ROW_SIZE);
            free(pager->pages[count]);
            pager->pages[count] = NULL;
        }
    }
    
    int res = close(pager->fileDescriptor);
    if (res == -1){
        printf("Error: Unable to close file.\n");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < TABLE_MAX_PAGES; i++){
        void* page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

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


void serializeRow(Row* source, void* target){
    memcpy( target,
            &source->id,
            sizeof(int));
    for (int i = 0; i < source->colCount; i++){
        memcpy( target +sizeof(int) +( (COL_WIDTH+1) *i),
                &source->col[i],
                sizeof(char) * (COL_WIDTH+1) );
    }
}

void deserializeRow(void* source, Row* target){
    memcpy(&target->id,
           source,
           sizeof(int));
    for (int i = 0; i < target->colCount; i++){
        memcpy(&target->col[i],
               source +sizeof(int) +( (COL_WIDTH+1) *i),
               sizeof(char) * (COL_WIDTH+1) );
    }
}

void* indexRow(Table* table, uint32_t row){
    uint32_t pgNr  = row / ROWS_PER_PAGE;
    void* page = getPage(table->pager, pgNr);
    
    uint32_t offset = (row % ROWS_PER_PAGE) * ROW_SIZE;
    return page + offset;
}

Row* prepareRow(int cols){
    Row* temp = (Row*)malloc(sizeof(Row));
    temp->colCount = cols; // id col + 2 other columns
    
    // adjust col size of the line
    temp->col = (char**)malloc( sizeof(char*) * temp->colCount);
    // adjust space for each col
    for (int i = 0; i < temp->colCount; i++)
        temp->col[i] = (char*)malloc( sizeof(char) * (COL_WIDTH+1));
    
    return temp;
}

void freeRow(Row* row){
    for (int i = 0; i < row->colCount; i++)
        free(row->col[i]); // space for each col
    free(row->col); // cols
    free(row);
}

int insertRowToTable(inputBuffer* line, Table* table){
    Row* row = prepareRow(2); // 2 coloums required + id column
    
    char* instruction = strtok(line->buffer, " ");
    char* colId = strtok(NULL, " ");
    char* col2 = strtok(NULL, " ");
    char* col3 = strtok(NULL, " ");
    //int res = sscanf(line->buffer, "insert %d %s %s",
      //               &row->id, row->col[0], row->col[1]);
    
    if (instruction  == NULL || colId == NULL || col2 == NULL || col3 == NULL){
        free(row);
        return FAILED;
    }
    row->id = atoi(colId);
    if (row->id < 0){
        free(row);
        return ERR_ID_NOT_POSITIVE;
    }
    if (strlen(col2) > COL_WIDTH || strlen(col3) > COL_WIDTH){
        free(row);
        return ERR_STRING_TOO_LARGE;
    }
    strcpy(row->col[0], col2);
    strcpy(row->col[1], col3);
    
    //insert Row To Table
    serializeRow(row, indexRow(table, table->num_rows));
    table->num_rows++;
    
    free(row);
    return OK;
    
}

int selectfromTable(inputBuffer* line, Table* table){
    Row* row = prepareRow(2); // 2 columns requred + id coloum
    int res = sscanf(line->buffer, "select \n"); // Todo
    
    if (res < row->colCount + 1){
    //    free(row);
    //    return -1;
    }
    
    //retrieve Rows from Table
    for (uint32_t i = 0; i < table->num_rows; i++){
        deserializeRow(indexRow(table, i), row);
        printf("%d %s %s\n", row->id, row->col[0], row->col[1]);
    }
    
    free(row);
    return OK;
}

int createTable(inputBuffer* line, Table** tablePtr){
    // Quick solution ...
    char* temp = (char*)malloc(255);
    int res = sscanf(line->buffer, "create table %s", temp);
    if (res <= 0){
        free(temp);
        printf("Error: No file path detected.\n");
        return FAILED;
    }
    
    if (*tablePtr != NULL){
        closeDB(*tablePtr); // Todo proper way
    }
    
    *tablePtr = openDB(temp);
    free(temp);
    return OK;
}

int inputFromUser(inputBuffer* in){
    ssize_t inBytes = getline(&in->buffer, &in->length, stdin);
    if (inBytes <= 0){ // failed to input
        return FAILED;
    }
    
    // remember the trailing newline
    in->inputLength = inBytes - 1;
    in->buffer[inBytes - 1] = '\0';
    return OK;
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

int processCommand(inputBuffer* cmd, Table** tablePtr){
    Table* table = *tablePtr;
    if (strcmp(cmd->buffer, ".exit") == 0){
        // Exit
        freeBuffer(cmd);
        closeDB(table);
        exit(EXIT_SUCCESS);
    }
    if (strncmp(cmd->buffer, ".open", 5) == 0){
        // Quick solution ...
        char* line = (char*)malloc(255);
        int res = sscanf(cmd->buffer, ".open %s", line);
        if (res <= 0){
            free(line);
            printf("Error: No file path detected. ");
            return FAILED;
        }
        
        if (table != NULL){
            closeDB(table); // Todo proper way
        }
        printf("Openning...\n");
        *tablePtr = openDB(line);
        free(line);
        return OK;
    }
    
    // Todo add more commands
    return FAILED;
}

#define INSERT 1
#define SELECT 2
#define CREATE 3

int processStatement(inputBuffer* stmt, Table** table){
    if (strncmp(stmt->buffer, "insert", 6) == 0)
        execute(INSERT, stmt, table);
    else if (strncmp(stmt->buffer, "select", 6) == 0)
        execute(SELECT, stmt, table);
    else if (strncmp(stmt->buffer, "create", 6) == 0)
        execute(CREATE, stmt, table);
    else
        return FAILED;
    
    return OK;
}

void execute(int stmt, inputBuffer* line, Table** tablePtr){
    int res;
    Table* table = *tablePtr;
    switch (stmt){
        case INSERT:
            if (table == NULL){
                printf("Error! No table created.");
                break;
            }
            if (table->num_rows >= TABLE_MAX_ROWS){
                printf("Error! Table is already full");
                break;
            }
            printf("Inserting...");
            res = insertRowToTable(line, table);
            if (res == OK)
                printf("Successfully executed insert statement.");
            else if (res == ERR_STRING_TOO_LARGE)
                printf("Error! Length of string cannot exceed %d.", COL_WIDTH);
            else if (res == ERR_ID_NOT_POSITIVE)
                printf("Error! ID is required to be positive");
            else
                printf("Syntax Error!");
            break;
        case SELECT:
            if (table == NULL){
                printf("Error! No table created.");
                break;
            }
                
            printf("Selecting...\n");
            res = selectfromTable(line, table);
            if (res == OK)
                printf("Successfully executed select statement.");
            else
                printf("Sytax Error");
            break;
        case CREATE:
            printf("Creating...\n");
            res = createTable(line, tablePtr);
            if (res == OK)
                printf("Successfully created table");
            else
                printf("Sytax Error");
            break;
    }
    printf("\n");
}


