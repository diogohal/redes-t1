#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packages.h"
#include "fileHandler.h"

unsigned char *readArchive(FILE *file) {

    int count = 0;
    while(fgetc(file) != EOF)
        count++;

    printf("%d\n", count);
    rewind(file);
    unsigned char *fileContent = malloc(sizeof(unsigned char)*count + 1);
    if(!fileContent)
        return NULL;
    
    for(int i=0; i<count+1; i++)
        fscanf(file, "%c", &fileContent[i]);
    
    return fileContent;

}

void writeFile(unsigned char *string) {

    FILE *file = fopen("./backup/test.txt", "w");

    fprintf(file, "%s", string);

    fclose(file);

}

unsigned char *createString(root_t *root) {

    unsigned char *string = malloc((root->count-1)*DATA_SIZE + strlen(root->tail->message->data));
    string[0] = '\0';
    node_t *aux = root->head;
    while(aux) {
        strcat(string, aux->message->data);
        aux = aux->next;
    }

    return string;

}