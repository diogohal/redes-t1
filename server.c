#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <dirent.h>
#include "rawSocketConnection.h"
#include "packages.h"
#include "fileHandler.h"

void getCommand(int *command, char *input) {

    char *token = NULL;
    token = strtok(input, " ");
    token[strcspn(token, "\n")] = '\0';
    if(!strcmp(token, "backup"))
        *command = 0;
    else if(!strcmp(token, "backupdir"))
        *command = 1;
    else if(!strcmp(token, "exit"))
        *command = -1;
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
        fileName[strcspn(fileName, "\n")] = '\0';
        token = strtok(NULL, "/");
    }

}

int main() {

    // Variables and structs used in the client
    int server, new_socket, valread;
    int response = 0;
    int nackCount = 0;
    root_t *root = createRoot();
    node_t *auxNode = NULL;
    protocol_t *auxMessage = NULL;
    unsigned char *msg = NULL;
    int sequel = -1;
    protocol_t message;
    char cmd[100];
    char saveCmd[100];
    char dirPath[200]; char dirName[50]; char filePath[200]; char fileName[50];
    FILE *file = NULL;
    DIR *dirStream = NULL;
    server = rawSocketConnection("eno1");
    // Timeout setting
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    if(setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
        printf("Erro ao abrir o socket!\n");
        exit(1);
    }

    unsigned int parity = 0;
    ssize_t recvReturn;
    // Server running
    while(1) {
        // Receive message
        recvReturn = recv(server, &message, PROTOCOL_SIZE, 0);
        if(recvReturn == -1) {
            continue;
        }
        // ----- File -----
        if(message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8)) {
            // Server-Client talk
            parity = calculateParity(&message);
            if(parity != message.parity) {
                printf("Paridade errada. NACK enviado!\n");
                sendResponse(server, 0, 15, "", 0);
                continue;
            }
            if(message.type == 0) 
                sendResponse(server, 0, 13, "", 0);
            else if(message.type == 8)
                sendResponse(server, 0, 14, "", 0);
            // Create a list of messages
            receiveFileMessage(root, message);
        }
        
        // ----- Group file -----
        if(message.init_mark == 126 && message.type == 1) {
            printf("Recebendo Grupo de Arquivos\n");
            // Server-Client talk
            if(message.type == 1)
                sendResponse(server, 0, 13, "", 0);

            while(1) {
                recvReturn = recv(server, &message, PROTOCOL_SIZE, 0);
                if(recvReturn == -1)
                    continue;
                // File
                if(message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8 || message.type == 10)) {
                    // Server-Client talk
                    parity = calculateParity(&message);
                    if(parity != message.parity) {
                        nackCount++;
                        if(nackCount <= 3) {
                            printf("Paridade errada. NACK enviado!\n");
                            sendResponse(server, 0, 15, "", 0);
                            continue;
                        }
                    }
                    if(message.type == 0)
                        sendResponse(server, 0, 13, "", 0);
                    else if(message.type == 8)
                        sendResponse(server, 0, 14, "", 0);
                    else if(message.type == 10) {
                        printf("Fim Grupo de Arquivos\n");
                        break;
                    }
                    // Create a list of messages
                    receiveFileMessage(root, message);
                }
            }    
        }

        // ----- Rec backup -----
        if(message.init_mark == 126 && message.type == 2) {
            strcpy(filePath, "backup/");
            strcat(filePath, message.data);
            strcpy(fileName, message.data);
            printf("FILE PATH %s\n", filePath);
            file = fopen(filePath, "rb");
            if(!file) {
                sendResponse(server, 0, 12, "2", 1);
                printf("Erro enviado! Arquivo inexistente!\n");
                continue;
            }
            printf("Enviando %s arquivo!\n", fileName);
            sendFile(file, fileName, server, 0);
            fclose(file);
        }

        // ----- Rec backupgroup -----
        if(message.init_mark == 126 && message.type == 3) {
            strcpy(dirPath, message.data);
            sendGroupFiles(dirPath, server);
        }
    }
        
    
    return 0;

}