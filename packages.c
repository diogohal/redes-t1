#include <stdlib.h>
#include <stdio.h>
#include "packages.h"
#include <dirent.h>
#include "string.h"
#include "rawSocketConnection.h"
#include "fileHandler.h"

protocol_t *createMessage (unsigned int sequel, unsigned int type, unsigned char *data) {
    
    protocol_t *message = malloc(sizeof(protocol_t));
    
    int bit = 0, size = strlen(data);

    //alocate the init mark on the protocol
    for (int i = 0; i < 8; i++) {
        if (i == 0 || i == 7)
            message->init_mark |= (0 << i);
        else
            message->init_mark |= (1 << i);
    }


    //alocate the size of the message on the protocol
    message->size = size;
    message->sequel = sequel;
    message->type = type;
    strncpy(message->data, data, DATA_SIZE);
    message->parity = 0;

    return message;
}

protocol_t **createMessageBuffer (unsigned char *msg, int bufferSize, unsigned char *fileName) {
    
    char mensagem[DATA_SIZE];

    protocol_t **buf = malloc(sizeof(protocol_t) * (bufferSize));
    // First message is the backup type with it's filename
    buf[0] = createMessage(0, 0, fileName);
    // Last message is the ending file type
    buf[bufferSize-1] = createMessage(bufferSize-1, 9, "");

    for (int j = 0; j < bufferSize-2; j++) {
        for (int i = 0; i < DATA_SIZE; i++) {
            mensagem[i] = msg[i + (j*DATA_SIZE)];
        }
        buf[j+1] = createMessage(j+1, 8, mensagem);
    }
    return buf;
}

int calcBufferSize(unsigned char *msg) {

    int bufferSize = 0;
    int msgSize = strlen(msg);

    if (msgSize % DATA_SIZE == 0)
        bufferSize = msgSize / DATA_SIZE;
    else
        bufferSize = msgSize/DATA_SIZE + 1;

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

    return node;

}

void addNode(root_t *root, node_t *node) {

    node_t *aux = root->head;
    if(!aux) {
        root->head = node;
        root->tail = node;
    }
    else if(node->message->sequel < root->head->message->sequel) {
        node->next = root->head;
        root->head->before = node;
        root->head = node;
    }
    else if(node->message->sequel > root->tail->message->sequel) {
        root->tail->next = node;
        node->before = root->tail; 
        root->tail = node;
    }
    else {
        while(aux) {
            if(node->message->sequel < aux->message->sequel) {
                node->next = aux;
                node->before = aux->before;
                node->before->next = node;
                aux->before = node;
                break;
            }
            // bug treatment for loopback
            else if(node->message->sequel == aux->message->sequel)
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
            if (message.init_mark == 126 && message.type == 14 && message.sequel == messageBuffer[i]->sequel) {
                printf("ack recebido!\n");
                break;
            }
        }
    }

}

int sendACK(int raw, int sequel) {
    
    int result = 0;
    unsigned char buffer[67];
    protocol_t *ack = createMessage(sequel, 14, "");
    memcpy(buffer, ack, sizeof(protocol_t));
    result = send(raw, buffer, 67, 0);
    return result;

}

void sendFile(FILE *file, unsigned char *fileName, int sockfd) {

    unsigned char *msg = readArchive(file);
    int bufferSize = calcBufferSize(msg)+2;
    protocol_t **messageBuffer = createMessageBuffer(msg, bufferSize, fileName);
    sendMessage(messageBuffer, sockfd, bufferSize, sockfd);

}

void sendDirectory(unsigned char *dirPath, int socket) {

    DIR *dirStream = opendir(dirPath);
    char filePath[100];
    struct dirent *dirEntry = NULL;
    FILE *file = NULL;
    while((dirEntry = readdir(dirStream)) != NULL) {
        if(dirEntry->d_type == REGULAR_FILE) {
            strcpy(filePath, dirPath);
            strcat(filePath, "/");
            strcat(filePath, dirEntry->d_name);
            file = fopen(filePath, "r");
            if(!file)
                return;
            printf("\n\nEnviando %s\n", dirEntry->d_name);
            sendFile(file, dirEntry->d_name, socket);
            fclose(file);
        }
    }
    closedir(dirStream);

}

// ---------- RECEIVING FUNCTIONS ----------
int receiveFileMessage(root_t *root, protocol_t message) {

    protocol_t *auxMessage = createMessage(message.sequel, message.type, message.data);
    node_t *auxNode = createNode(auxMessage);
    printf("debugggg\n");
    addNode(root, auxNode);

    // Check for message ending. Needs a timestamp
    if(messageComplete(root)) {
        unsigned char *msg = createString(root);
        writeFile(msg, root->head->message->data);
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
    
    root->count = 0;

}