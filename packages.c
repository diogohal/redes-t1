#include <stdlib.h>
#include <stdio.h>
#include "packages.h"
#include <dirent.h>
#include "string.h"
#include "rawSocketConnection.h"
#include "fileHandler.h"

protocol_t *createMessage (unsigned int sequel, unsigned int type, unsigned char *data, int size) {
    protocol_t *message = malloc(sizeof(protocol_t));
    message->init_mark = 126;
    message->size = size;
    message->sequel = sequel;
    message->type = type;
    memcpy(message->data, data, size);
    message->parity = 0;
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

void printBuff (protocol_t **buf, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        printf("sequel = %d | type = %d | data = %s\n", buf[i]->sequel, buf[i]->type, buf[i]->data);
    }
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
    unsigned char buffer[67];
    protocol_t message;
    for(int i = 0; i < bufferSize; i++) {
        memcpy(buffer, messageBuffer[i], sizeof(protocol_t));
        send(socket, buffer, 67, 0);
        printf("Mensagem enviada!\n");
        // Doesn't need to wait for ack response
        if(i == bufferSize-1)
            return;
        while (1) {
            recv(raw, &message, 67, 0);
            if (message.init_mark == 126 && message.type == 14 && i > 0) {
                printf("ack recebido!\n");
                break;
            } else if (message.init_mark == 126 && message.type == 13 && i == 0) {
                printf("OK recebido!\n");
                break;
            }
        }
    }
}

int sendResponse(int raw, int sequel, int type, unsigned char *data, int size) {
    int result = 0;
    unsigned char buffer[67];
    protocol_t *ack = createMessage(sequel, type, data, size);
    memcpy(buffer, ack, sizeof(protocol_t));
    result = send(raw, buffer, 67, 0);
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

void sendDirectory(unsigned char *dirPath, int socket) {
    DIR *dirStream = opendir(dirPath);
    char filePath[100];
    int sequel = 0;
    struct dirent *dirEntry = NULL;
    FILE *file = NULL;
    sendResponse(socket, sequel, 1, dirPath, strlen(dirPath)+1);
    sequel++;
    while((dirEntry = readdir(dirStream)) != NULL) {
        if(dirEntry->d_type == REGULAR_FILE) {
            strcpy(filePath, dirPath);
            strcat(filePath, "/");
            strcat(filePath, dirEntry->d_name);
            file = fopen(filePath, "r");
            if(!file)
                return;
            printf("\n\nEnviando %s\n", dirEntry->d_name);
            sequel = sendFile(file, dirEntry->d_name, socket, sequel);
            printf("Arquivo enviado!!\n\n");
            fclose(file);
        }
    }
    sendResponse(socket, sequel, 10, "", 0);
    closedir(dirStream);
}

// ---------- RECEIVING FUNCTIONS ----------
int receiveFileMessage(root_t *root, protocol_t message) {
    int sequel = 0;
    if(root->tail && root->tail->sequel >= 63)
        sequel = root->tail->sequel + 1;
    else
        sequel = message.sequel;
    protocol_t *auxMessage = createMessage(sequel, message.type, message.data, message.size);
    node_t *auxNode = createNode(auxMessage);
    auxNode->sequel = sequel;
    addNode(root, auxNode);
    printf("SEQUENCIA ADICIONADA = %d\n", sequel);
    // Check for message ending. Needs a timestamp
    if(messageComplete(root)) {
        int fileSize = 0;
        unsigned char *msg = createString(root, &fileSize);
        writeFile(msg, root->head->message->data, fileSize);
        destroyNodes(root);
        printf("Arquivo escrito!\n");
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