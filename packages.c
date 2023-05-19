#include <stdlib.h>
#include <stdio.h>
#include "packages.h"
#include "string.h"
#include "rawSocketConnection.h"

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
    message->size = size = size;
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

protocol_t **createMessageBuffer (unsigned char *msg, int bufferSize) {
    
    int modMsg = strlen(msg) % DATA_SIZE;
    char mensagem[DATA_SIZE];

    protocol_t **buf = malloc(sizeof(protocol_t) * bufferSize);

    for (int j = 0; j < bufferSize; j++) {
        for (int i = 0; i < DATA_SIZE; i++) {
            mensagem[i] = msg[i + (j*DATA_SIZE)];
        }
        buf[j] = createMessage(j, 0, mensagem);
    }

    return buf;
}

void sendMessage(protocol_t **messageBuffer, int socket, int bufferSize) {

    unsigned char buffer[67];
    for(int i = 0; i < bufferSize; i++) {
        // protocolToBuffer(buffer, messageBuffer[i]);
        memcpy(buffer, messageBuffer[i], sizeof(protocol_t));
        send(socket, buffer, 68, 0);
    }

}

void printBuff (protocol_t **buf, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        printf("init_mark: %d size: %d sequel: %d type: %d data: %s, parity: %d \n" , buf[i]->init_mark, buf[i]->size, buf[i]->sequel, buf[i]->type, buf[i]->data, buf[i]->parity);
    }
}