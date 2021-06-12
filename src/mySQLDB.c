#include "mySQLDB.h"
#include "storage.h"

#define ERR_DUPLICATE_KEY -4
#define ERR_ID_NOT_POSITIVE -3
#define ERR_STRING_TOO_LARGE -2
#define FAILED -1
#define OK 1


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
    pager->pageCount = (len / PAGE_SIZE);
    
    if (len % PAGE_SIZE != 0){
        printf("Db file is not whole. File corrupted.\n");
        exit(EXIT_FAILURE);
    }
    
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
            lseek(pager->fileDescriptor, (pageCount-1) * PAGE_SIZE, SEEK_SET);
            ssize_t bytes = read(pager->fileDescriptor, page, PAGE_SIZE);
            if (bytes == -1){
                printf("Error: Couldn't read file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        
        pager->pages[pgNr] = page;
    }
    
    if (pgNr >= pager->pageCount)
        pager->pageCount = pgNr +1;
    
    return pager->pages[pgNr];
}
uint32_t getEmptyPage(Pager* pager){
    // Todo, recycle free pages
    return pager->pageCount;
}

Table* openDB(const char* file){
    // Initialize table
    Table* table = malloc(sizeof(Table));
    table->pager = initPager(file);
    table->rootPage = 0;
    
    if (table->pager->pageCount == 0){
        // Empty, initialize page 0 as leaf
        void* root = getPage(table->pager, 0);
        initLeafNode(root);
        setIsRootNode(root, true);
    }
    
    return table;
}

void updateDisk(Pager* pager, uint32_t pgNr){
    if (pager->pages[pgNr] == NULL){
        printf("Error: Attempting to store a null page.\n");
        exit(EXIT_FAILURE);
    }
    
    off_t pos = lseek( pager->fileDescriptor, pgNr * PAGE_SIZE, SEEK_SET);
    if (pos == -1){
        printf("Error: Unable to store page - failure to seek.\n");
        exit(EXIT_FAILURE);
    }
    
    ssize_t bytes  = write(pager->fileDescriptor, pager->pages[pgNr], PAGE_SIZE);
    
    if (bytes == -1){
        printf("Error: Failed to write to disk - %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void closeDB(Table* table){
    Pager* pager = table->pager;
    for (size_t i = 0; i < pager->pageCount; i++){
        if (pager->pages[i] == NULL)
            continue;
        
        updateDisk(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
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

TableCursor* tableStart(Table* table){
    TableCursor* cursor = (TableCursor*)malloc(sizeof(TableCursor));
    cursor->table = table;
    cursor->cellNr = 0;
    cursor->pgNr = table->rootPage;
    
    void* root = getPage(table->pager, table->rootPage);
    cursor->endOfTable  = (*getLeafCellCount(root) == 0);
    
    return cursor;
}

TableCursor* tableEnd(Table* table){
    TableCursor* cursor = (TableCursor*)malloc(sizeof(TableCursor));
    cursor->table = table;
    cursor->pgNr = table->rootPage;
    
    void* root = getPage(table->pager, table->rootPage);
    cursor->cellNr = *getLeafCellCount(root);
    
    cursor->endOfTable  = true;
    return cursor;
}

TableCursor* findFromTable(Table* table, uint32_t key){
    uint32_t rootPageNr = table->rootPage;
    void* node = getPage(table->pager, rootPageNr);
    
    if (getNodeType(node) == Leaf)
        return findFromLeaf(table, rootPageNr, key);
    else {
        return nodeFind(table, rootPageNr, key);
    }
}

void next(TableCursor* cursor){
    void* node = getPage(cursor->table->pager, cursor->pgNr);
    cursor->cellNr++;
    
    if (cursor->cellNr >= *getLeafCellCount(node))
        cursor->endOfTable = true;
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
            &( source->id ),
            sizeof(int));
    for (int i = 0; i < source->colCount; i++){
        memcpy( target +sizeof(int) +( (COL_WIDTH+1) *i),
                source->col[i],
                sizeof(char) * (COL_WIDTH+1) );
    }
}

void deserializeRow(void* source, Row* target){
    memcpy(&( target->id ),
           source,
           sizeof(int));
    for (int i = 0; i < target->colCount; i++){
        memcpy(target->col[i],
               source +sizeof(int) +( (COL_WIDTH+1) *i),
               sizeof(char) * (COL_WIDTH+1) );
    }
}

void* indexValue(TableCursor* cursor){
    void* page = getPage(cursor->table->pager, cursor->pgNr);
    
    return getLeafValue(page, cursor->cellNr);
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
    void* node = getPage(table->pager, table->rootPage);
    
    uint32_t cellCount = (*getLeafCellCount(node));
    /*if (cellCount >= LeafMaxCells){
        free(row);
        return ERR_TABLE_FULL;
    }*/
    
    char* instruction = strtok(line->buffer, " ");
    char* colId = strtok(NULL, " ");
    char* col2 = strtok(NULL, " ");
    char* col3 = strtok(NULL, " ");
    
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
    
    // Check if key already exists
    TableCursor* cursor = findFromTable(table, row->id);
    
    if (cursor->cellNr < cellCount){
        uint32_t locatedKey = *getLeafKey(node, cursor->cellNr);
        if (locatedKey == row->id){
            free(row);
            free(cursor);
            return ERR_DUPLICATE_KEY;
        }
    }
    //insert Row To Table
    insertLeaf(cursor, row);
    
    free(row);
    free(cursor);
                 
    return OK;
    
}

int selectfromTable(inputBuffer* line, Table* table){
    Row* row = prepareRow(2); // 2 columns requred + id coloum
    TableCursor* cursor  = tableStart(table);
    
    int res = sscanf(line->buffer, "select \n"); // Todo
    
    if (res < row->colCount + 1){
        // bcoz still need to do above line
        // let it search whole table for now
    }
    
    //retrieve Rows from Table
    while (!(cursor->endOfTable)) {
        deserializeRow(indexValue(cursor), row);
        printf("%d %s %s\n", row->id, row->col[0], row->col[1]);
        next(cursor);
    }
    
    free(row);
    free(cursor);
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
        if (table != NULL)
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
            printf("Inserting...\n");
            res = insertRowToTable(line, table);
            if (res == OK)
                printf("Successfully executed insert statement.");
            else if (res == ERR_STRING_TOO_LARGE)
                printf("Error! Length of string cannot exceed %d.", COL_WIDTH);
            else if (res == ERR_ID_NOT_POSITIVE)
                printf("Error! ID is required to be positive");
            else if (res == ERR_DUPLICATE_KEY)
                printf("Error! Cannot insert duplicate keys");
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


