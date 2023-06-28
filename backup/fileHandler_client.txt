#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packages.h"
#include "fileHandler.h"

// ---------- READ FUNCTIONS ----------
unsigned char *readArchive(FILE *file, int* outFileSize) {
    int fileSize = 0;
    unsigned char *fileContent = NULL;
    while(fgetc(file) != EOF)
        fileSize++;
    rewind(file);
    fileContent = malloc(sizeof(unsigned char)*fileSize);
    fread(fileContent, fileSize, 1, file);
    *outFileSize = fileSize;
    return fileContent;
}

// ---------- WRITE FUNCTIONS ----------
void writeFile(unsigned char *string, unsigned char *fileName, int fileSize) {
    char filePath[200];
    strcpy(filePath, "./backup/");
    strcat(filePath, fileName);
    FILE *file = fopen(filePath, "wb");
    fwrite(string, fileSize, 1, file);
    fclose(file);
}

unsigned char *createString(root_t *root, int* outFileSize) {
    unsigned char *string = malloc((root->count-1)*DATA_SIZE + root->tail->message->size);
    node_t *aux = root->head->next;
    *outFileSize = 0;
    while(aux) {
        memcpy(string + (*outFileSize), aux->message->data, aux->message->size);
        *outFileSize += aux->message->size;
        aux = aux->next;
    }
    return string;
}

int messageComplete(root_t *root) {
    node_t *aux = root->head;
    int count = aux->sequel;
    while(aux) {
        if(count != aux->sequel)
            return 0;
        else if(aux->message->type == 9)
            return 1;
        aux = aux->next;
        count++;
    }
    return 0;
}