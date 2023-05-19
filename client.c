#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"
#include "packages.h"

#define DEST_PORT 8080
#define DEST_IP "172.18.0.1"

int calcBufferSize(unsigned char *msg) {

    int bufferSize = 0;
    int msgSize = strlen(msg);

    if (msgSize % DATA_SIZE == 0)
        bufferSize = msgSize / DATA_SIZE;
    else
        bufferSize = msgSize/DATA_SIZE + 1;

    return bufferSize;

}

int main() {

    int sockfd = 0;
    int bufferSize = 0;
    struct sockaddr_in dest_addr;
    protocol_t **messageBuffer = NULL;
    int running = 1;
    int count = 0;
    unsigned char *msg = "EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL EU ODEIO O NATAEL!!!";
    // unsigned char *msg = "Fernado Diogo Redes Fernado Diogo Redes Fernado Diogo Redes Fernado Diogo Redes Fernado Diogo Redes Fernado Diogo Redes Fernandoo";
    char cmd[100];
    sockfd = rawSocketConnection("enp0s25");
    
    while (running) {
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = '\0';
        if (!strcmp(cmd, "backup")) {
            bufferSize = calcBufferSize(msg);
            messageBuffer = createMessageBuffer(msg, bufferSize);
            printBuff (messageBuffer, bufferSize);
            sendMessage(messageBuffer, sockfd, bufferSize);

        } else if (!strcmp(cmd, "exit")) {
            running = 0;
        }
    }

    return 0;

}