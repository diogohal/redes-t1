#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include <string.h>
#include "packages.h"
#include "string.h"
#include "rawSocketConnection.h"
#include "fileHandler.h"

unsigned int calculateParity(protocol_t *message) {
    unsigned int parity = 0;
    for(int i=0; i<message->size; i++)
        parity ^= message->data[i];
    return parity;
}

protocol_t *createMessage (unsigned int sequel, unsigned int type, unsigned char *data, int size) {
    protocol_t *message = malloc(sizeof(protocol_t));
    message->init_mark = 126;
    message->size = size;
    message->sequel = sequel;
    message->type = type;
    memcpy(message->data, data, size);
    message->parity = calculateParity(message);
    return message;
}

protocol_t **createMessageBuffer (unsigned char *msg, int fileSize, int bufferSize, unsigned char *fileName, int sequel) {
    char mensagem[DATA_SIZE];
    protocol_t **buf = malloc(sizeof(protocol_t) * (bufferSize));
    // First message is the backup type with it's filename
    buf[0] = createMessage(sequel, 0, fileName, strlen(fileName)+1);
    sequel++;
    for (int j = 0; j < bufferSize-2; j++) {
        int c = 0;
        for (int i = 0; i < DATA_SIZE; i++) if (i + (j*DATA_SIZE) < fileSize) {
            mensagem[i] = msg[i + (j*DATA_SIZE)];
            c++;
        }
        buf[j+1] = createMessage((sequel++), 8, mensagem, c);
    }
    // Last message is the ending file type
    buf[bufferSize-1] = createMessage(sequel, 9, "", 0);
    return buf;
}

int calcBufferSize(int fileSize) {
    int bufferSize = 0;
    if (fileSize % DATA_SIZE == 0)
        bufferSize = fileSize / DATA_SIZE;
    else
        bufferSize = fileSize/DATA_SIZE + 1;
    return bufferSize;
}

root_t *createRoot() {
    root_t *root = malloc(sizeof(root_t));
    if(!root)
        return NULL;
    root->head = NULL;
    root->tail = NULL;
    root->count = 0;
    return root;
}

node_t *createNode(protocol_t *message) {
    node_t *node = malloc(sizeof(node_t));
    if(!node)
        return NULL;
    node->message = message;
    node->before = NULL;
    node->next = NULL;
    node->sequel = 0;
    return node;
}

void addNode(root_t *root, node_t *node) {
    node_t *aux = root->head;
    if(!aux) {
        root->head = node;
        root->tail = node;
    }
    else if(node->sequel < root->head->sequel) {
        node->next = root->head;
        root->head->before = node;
        root->head = node;
    }
    else if(node->sequel > root->tail->sequel) {
        root->tail->next = node;
        node->before = root->tail; 
        root->tail = node;
    }
    else {
        while(aux) {
            if(node->sequel < aux->sequel) {
                node->next = aux;
                node->before = aux->before;
                node->before->next = node;
                aux->before = node;
                break;
            }
            // bug treatment for loopback
            else if(node->sequel == aux->sequel)
                return;
            aux = aux->next; 
        }
    }
    root->count++;
}

// ---------- SEND FUNCTIONS ----------
void sendMessage(protocol_t **messageBuffer, int socket, int bufferSize, int raw) {
    unsigned char buffer[PROTOCOL_SIZE];
    protocol_t message;
    ssize_t recvReturn;
    for(int i = 0; i < bufferSize; i++) {
        memcpy(buffer, messageBuffer[i], sizeof(protocol_t));
        send(socket, buffer, PROTOCOL_SIZE, 0);
        // Doesn't need to wait for ack response
        if(i == bufferSize-1)
            return;
        while (1) {
            recvReturn = recv(raw, &message, PROTOCOL_SIZE, 0);
            if(recvReturn == -1) {
                printf("Timeout! Esperando 2 segundos...\n");
                send(socket, buffer, PROTOCOL_SIZE, 0);
                continue;
            }
            if (message.init_mark == 126 && message.type == 14 && i > 0)
                break;
            else if (message.init_mark == 126 && message.type == 13 && i == 0)
                break;
            else if (message.init_mark == 126 && message.type == 15)
                send(socket, buffer, PROTOCOL_SIZE, 0);
        }
    }
}

// Send group files
void sendGroupFiles(unsigned char *groupFiles, int socket) {
    // Glob setting
    char **found;
    glob_t gstruct;
    int r;
    r = glob(groupFiles, GLOB_ERR, NULL, &gstruct);
    if(r != 0) {
        printf("Não encontrou nada!\n");
        return;
    }
    found = gstruct.gl_pathv;

    // Variables setting
    int sequel = 0;
    FILE *file = NULL;
    sendResponse(socket, sequel, 1, "", 0);
    sequel++;
    char filePath[100];
    char fileName[100];
    char *token;

    // Send each found
    while(*found) {
        strcpy(filePath, *found);
        file = fopen(filePath, "rb");
        if(!file)
            return;
        // Get file name
        token = strtok(filePath, "/");
        while(token != NULL) {
            strcpy(fileName, token);
            token = strtok(NULL, "/");
        }
        printf("Enviando %s\n", *found);
        sequel = sendFile(file, fileName, socket, sequel);
        printf("Arquivo enviado!!\n\n");
        found++;
        fclose(file);
    }
    sendResponse(socket, sequel, 10, "", 0);
}

int sendResponse(int raw, int sequel, int type, unsigned char *data, int size) {
    int result = 0;
    unsigned char buffer[PROTOCOL_SIZE];
    protocol_t *ack = createMessage(sequel, type, data, size);
    memcpy(buffer, ack, sizeof(protocol_t));
    result = send(raw, buffer, PROTOCOL_SIZE, 0);
    return result;
}

int sendFile(FILE *file, unsigned char *fileName, int sockfd, int sequel) {
    int fileSize;
    unsigned char *msg = readArchive(file, &fileSize);
    int bufferSize = calcBufferSize(fileSize)+2;
    protocol_t **messageBuffer = createMessageBuffer(msg, fileSize, bufferSize, fileName, sequel);
    sendMessage(messageBuffer, sockfd, bufferSize, sockfd);
    return sequel+bufferSize;
}

// ---------- RECEIVING FUNCTIONS ----------
int receiveFileMessage(root_t *root, protocol_t message) {
    int sequel = 0;
    if(root->tail && root->tail->sequel >= DATA_SIZE)
        sequel = root->tail->sequel + 1;
    else
        sequel = message.sequel;
    protocol_t *auxMessage = createMessage(sequel, message.type, message.data, message.size);
    node_t *auxNode = createNode(auxMessage);
    auxNode->sequel = sequel;
    addNode(root, auxNode);
    // Check for message ending. Needs a timestamp
    if(messageComplete(root)) {
        int fileSize = 0;
        unsigned char *msg = createString(root, &fileSize);
        writeFile(msg, root->head->message->data, fileSize);
        printf("Arquivo %s escrito!\n", root->head->message->data);
        destroyNodes(root);
        return 1;
    }
    return 0;
}

// ---------- DESTROY FUNCTIONS ----------
void destroyNodes(root_t *root) {
    node_t *aux = root->head;
    node_t *del = NULL;
    while(aux) {
        del = aux;
        aux = aux->next;
        free(del->message);
        free(del);
    }
    root->head = NULL;
    root->tail = NULL;
    root->count = 0;
}