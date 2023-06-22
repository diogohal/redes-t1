#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <stdio.h>
#include <ctype.h>
#include "packages.h"

unsigned char *readArchive(FILE *file, int* outFileSize);
unsigned char *createString(root_t *root, int* outFileSize);
void writeFile(unsigned char *string, int fileSize, unsigned char *fileName);
int messageComplete(root_t *root);

#endif