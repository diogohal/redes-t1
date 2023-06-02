#include <stdlib.h>
#include <stdio.h>
#include "packages.h"
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

void protocolToBuffer (unsigned char buffer[68], protocol_t *message) {
    
    unsigned char *bufferPtr = buffer;

    *bufferPtr++ = (unsigned char) (message->init_mark);
    *bufferPtr++ = (unsigned char) ((message->size << 2) | (message->sequel >> 4));
    *bufferPtr++ = (unsigned char) ((message->init_mark << 4) | message->type);

    for (int i = 0; i < DATA_SIZE; i++)
        *bufferPtr++ = message->data[i];
    
    *bufferPtr++ = (unsigned char) (message->parity);

    return;
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
        printf("init_mark: %d size: %d sequel: %d type: %d data: %s, parity: %d \n" , buf[i]->init_mark, buf[i]->size, buf[i]->sequel, buf[i]->type, buf[i]->data, buf[i]->parity);
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
        // protocolToBuffer(buffer, messageBuffer[i]);
        memcpy(buffer, messageBuffer[i], sizeof(protocol_t));
        
        send(socket, buffer, 67, 0);
        printf("Mensagem enviada!\n");
        
        while (1) {
            recv(raw, &message, 67, 0);
            if (message.init_mark == 126 && message.type == 14)
                break;
        }
    }

}

int sendACK(int raw) {
    
    int result = 0;
    unsigned char buffer[67];
    protocol_t *ack = createMessage(0, 14, "");
    memcpy(buffer, ack, sizeof(protocol_t));
    result = send(raw, buffer, 67, 0);
    return result;

}

void sendFile(unsigned char *msg, unsigned char *fileName, int sockfd) {

    int bufferSize = calcBufferSize(msg)+2;
    protocol_t **messageBuffer = createMessageBuffer(msg, bufferSize, fileName);
    sendMessage(messageBuffer, sockfd, bufferSize, sockfd);

}


// ---------- RECEIVING FUNCTIONS ----------
int receiveFileMessage(root_t *root, protocol_t message) {

    protocol_t *auxMessage = createMessage(message.sequel, message.type, message.data);
    node_t *auxNode = createNode(auxMessage);
    addNode(root, auxNode);
    // Check for message ending. Needs a timestamp
    if(messageComplete(root)) {
        unsigned char *msg = createString(root);
        writeFile(msg, root->head->message->data);
        destroyNodes(root);
        printf("Arquivo escrito!\n");
        return 1;
    }

}