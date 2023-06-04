#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <stdio.h>
#include "packages.h"

unsigned char *readArchive(FILE *file);
unsigned char *createString(root_t *root);
void writeFile(unsigned char *string, unsigned char *fileName);
int messageComplete(root_t *root);

#endif