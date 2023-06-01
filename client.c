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

void getCommand(int *command, char *input) {

    char *token = NULL;
    token = strtok(input, " ");
    token[strcspn(token, "\n")] = '\0';
    if(!strcmp(token, "backup"))
        *command = 1;
    else if(!strcmp(token, "exit"))
        *command = 0;
    else
        *command = 404;

}

void getDirPath(char *dirPath, char *input) {

    char *token = NULL;
    token = strtok(input, " ");
    token = strtok(NULL, " ");
    if(token) {
        strcpy(dirPath, token);
        dirPath[strcspn(dirPath, "\n")] = '\0';
    }
}

void getFileName(char *fileName, char *input) {

    char *token = NULL;
    token = strtok(input, "/");
    while(token) {
        strcpy(fileName, token);
        token = strtok(NULL, "/");
    }

}

int main() {

    // Variables and structs used in the client
    int sockfd = 0;
    int bufferSize = 0;
    struct sockaddr_in dest_addr;
    char *token = NULL;
    FILE *file = NULL;
    size_t fileSize;
    protocol_t **messageBuffer = NULL;
    int running = 1;
    int count = 0;
    unsigned char *msg = NULL;
    char cmd[100];
    char saveCmd[100];
    sockfd = rawSocketConnection("lo");
    int command = -1; char dirPath[200]; char fileName[50];
    
    // Client running
    while (running) {
        // ----- Get terminal command -----
        command = -1;
        fgets(cmd, sizeof(cmd), stdin);
        strcpy(saveCmd, cmd);
        getCommand(&command, cmd);
        strcpy(cmd, saveCmd);
        // ----- Commands execution -----
        // 1) 1 file backup
        if (command == 1) {
            getDirPath(dirPath, cmd);
            file = fopen(dirPath, "rb");
            if(!file) {
                printf("Arquivo ou diretório inexistente!\n");
                continue;
            }
            // Get filename
            strcpy(cmd, saveCmd);
            getFileName(fileName, cmd);
            msg = readArchive(file);
            bufferSize = calcBufferSize(msg)+2;
            messageBuffer = createMessageBuffer(msg, bufferSize, fileName);
            sendMessage(messageBuffer, sockfd, bufferSize);
            fclose(file);

        // 0) Exit
        } else if (command == 0)
            running = 0;
        
        // 404) Error
        else if(command = 404)
            printf("Comando não encontrado!\n");
    }

    return 0;

}