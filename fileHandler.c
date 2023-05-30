#include <stdio.h>
#include <stdlib.h>
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