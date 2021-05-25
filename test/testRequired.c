#include "test.h"

int testFileStream;
char* testFilePath = "bin/write.txt";

int replaceStream(){
    system("echo '' > bin/write.txt"); //  file exists, is empty
    testFileStream = open(testFilePath, O_WRONLY); // open a file
    
    int backup = dup(STDOUT_FILENO); // make backup of stdout
    
    fflush(stdout); //clean everything first
    
    dup2(testFileStream, STDOUT_FILENO ); // redirect printf() into file
    return backup;
}

void restoreStream(int backup){
    fflush(stdout); //clean everything first
    dup2(backup, STDOUT_FILENO);
    close(testFileStream);
}

char** loadFromFile(){
    FILE *file;
    file = fopen(testFilePath, "r");
    
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
char** createBuf(int size){
    return (char**)malloc(sizeof(char*) * (size + 1) ); // final NULL to signal end of array
}

char** appendBuf(char*** buff, int idx, char* str){
    int len = strlen(str);
    
    // final NULL to signal end of array
    char** buffer = *buff;
    buffer[idx] = (char*)realloc(buffer[idx], len+1);
    strncpy(buffer[idx], str, len);
    buffer[idx][len] = '\0';

    buffer[idx+1] = NULL;
    
    return buffer;
}

void freeBuff(char** ptr){
    if (ptr == NULL)
        return;
    for (int i = 0; ptr[i] != NULL; i++){
        free(ptr[i]);
    }
    free(ptr);
}

void show(char** expected, char** result){
    // RESULTS
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
}

