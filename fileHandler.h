#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <stdio.h>

unsigned char *readArchive(FILE *file);
unsigned char *createString(root_t *root);
void writeFile(unsigned char *string);

#endif