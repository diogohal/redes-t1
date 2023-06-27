#ifndef PACKAGES_H
#define PACKAGES_H
#define DATA_SIZE 63
#define PROTOCOL_SIZE 67
#define REGULAR_FILE 8
#define REGULAR_FOLDER 4
#include <dirent.h>

struct protocol {
    unsigned int init_mark : 8;
    unsigned int size : 6;
    unsigned int sequel : 6;
    unsigned int type : 4;
    unsigned char data[DATA_SIZE];
    unsigned int parity : 8;
}; typedef struct protocol protocol_t;


typedef struct node {
    protocol_t *message;
    struct node *next;
    struct node *before;
    int sequel;
} node_t;

typedef struct root {
    node_t *head;
    node_t *tail;
    int count;
} root_t;

unsigned int calculateParity(protocol_t *message);

protocol_t *createMessage (unsigned int sequel, unsigned int type, unsigned char *data, int size);

protocol_t **createMessageBuffer (unsigned char *msg, int fileSize, int bufferSize, unsigned char *fileName, int sequel);

void sendMessage(protocol_t **messageBuffer, int socket, int bufferSize, int raw);

void printBuff (protocol_t **buf, int bufferSize);

root_t *createRoot();

node_t *createNode(protocol_t *message);

void addNode(root_t *root, node_t *node);

int sendResponse(int raw, int sequel, int type, unsigned char *data, int size);

int sendFile(FILE *file, unsigned char *fileName, int sockfd, int sequel);

int receiveFileMessage(root_t *root, protocol_t message);

void sendDirectory(unsigned char *dirPath, int socket);

void destroyNodes(root_t *root);

void sendGroupFiles(unsigned char *groupFiles, int socket);

#endif