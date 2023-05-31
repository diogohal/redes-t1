#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"
#include "fileHandler.h"
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

    // Variables and structs used in the client
    int sockfd = 0;
    int bufferSize = 0;
    struct sockaddr_in dest_addr;
    char *token = NULL;
    char *fileName = "musica.txt";
    FILE *file = fopen("data/babababy.txt" , "r");
    protocol_t **messageBuffer = NULL;
    int running = 1;
    int count = 0;
    unsigned char *msg = readArchive(file);
    char cmd[100];
    sockfd = rawSocketConnection("lo");
    
    // Client running
    while (running) {
        // Get terminal command
        fgets(cmd, sizeof(cmd), stdin);
        token = strtok(cmd, " ");
        printf("%s\n", token);
        cmd[strcspn(cmd, "\n")] = '\0';
        // Commands execution
        // 1 file backup
        if (!strcmp(token, "backup")) {
            bufferSize = calcBufferSize(msg)+2;
            messageBuffer = createMessageBuffer(msg, bufferSize, fileName);
            sendMessage(messageBuffer, sockfd, bufferSize);
        // Exit
        } else if (!strcmp(cmd, "exit"))
            running = 0;
    }

    return 0;

}