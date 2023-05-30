#ifndef PACKAGES_H
#define PACKAGES_H
#define DATA_SIZE 63

struct protocol {
    unsigned int init_mark : 8;
    unsigned int size : 6;
    unsigned int sequel : 6;
    unsigned int type : 4;
    unsigned char data[63] ;
    unsigned int parity : 8;
}; typedef struct protocol protocol_t;


typedef struct node {
    protocol_t *message;
    struct node *next;
    struct node *before;
} node_t;

typedef struct root {
    node_t *head;
    node_t *tail;
    int count;
} root_t;

protocol_t *createMessage (unsigned int sequel, unsigned int type, unsigned char *data);

void protocolToBuffer (unsigned char buffer[68], protocol_t *protocol);

protocol_t **createMessageBuffer (unsigned char *msg, int bufferSize);

void sendMessage(protocol_t **messageBuffer, int socket, int bufferSize);

void printBuff (protocol_t **buf, int bufferSize);

root_t *createRoot();

node_t *createNode(protocol_t *message);

void addNode(root_t *root, node_t *node);

#endif