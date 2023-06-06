#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packages.h"
#include "fileHandler.h"

// ---------- READ FUNCTIONS ----------
unsigned char *readArchive(FILE *file) {

    int count = 0;
    while(fgetc(file) != EOF)
        count++;

    rewind(file);
    unsigned char *fileContent = malloc(sizeof(unsigned char)*count + 1);
    if(!fileContent)
        return NULL;
    
    for(int i=0; i<count+1; i++)
        fscanf(file, "%c", &fileContent[i]);
    
    return fileContent;

}

// ---------- WRITE FUNCTIONS ----------
void writeFile(unsigned char *string, unsigned char *fileName) {

    char filePath[200];
    strcpy(filePath, "./backup/");
    strcat(filePath, fileName);
    FILE *file = fopen(filePath, "w");

    fprintf(file, "%s", string);

    fclose(file);

}

unsigned char *createString(root_t *root) {

    unsigned char *string = malloc((root->count-1)*DATA_SIZE + strlen(root->tail->message->data));
    string[0] = '\0';
    node_t *aux = root->head->next;
    while(aux) {
        strcat(string, aux->message->data);
        aux = aux->next;
    }

    return string;

}

int messageComplete(root_t *root) {

    node_t *aux = root->head;
    int count = aux->message->sequel;

    while(aux) {
        if(count != aux->message->sequel)
            return 0;
        else if(aux->message->type == 9)
            return 1;
        aux = aux->next;
        count++;
    }

    return 0;

}