#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    root_t *root = createRoot();
    node_t *auxNode = NULL;
    protocol_t *auxMessage = NULL;
    unsigned char *msg = NULL;
    int sequel = -1;
    protocol_t message;
    char cmd[100];
    char saveCmd[100];
    char dirPath[200]; char filePath[200]; char fileName[50];
    FILE *file = NULL;
    DIR *dirStream = NULL;
    server = rawSocketConnection("eno1");

    // Server running
    while(1) {
        // Receive message
        recv(server, &message, 67, 0);

        // ----- File -----
        if(message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8)) {
            printf("Recebi mensagem %d | data = %s\n", message.sequel, message.data);
            // Server-Client talk
            if(message.type == 0) {
                sendResponse(server, 0, 13, "", 0);
                printf("OK ENVIADO!\n");
            }
            else if(message.type == 8) {
                sendResponse(server, 0, 14, "", 0);
                printf("ACK ENVIADO!\n");
            }
            // Create a list of messages
            receiveFileMessage(root, message);
        }
        
        // ----- Group file -----
        if(message.init_mark == 126 && message.type == 1) {
            printf("Recebendo Grupo de Arquivos\n");
            // Server-Client talk
            if(message.type == 1) {
                sendResponse(server, 0, 13, "", 0);
                printf("OK ENVIADO!\n");
            }

            while(1) {
                recv(server, &message, 67, 0);
                // File
                if(message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8 || message.type == 10)) {
                    printf("Recebi mensagem %d | data = %s\n", message.sequel, message.data);
                    // Server-Client talk
                    if(message.type == 0) {
                        sendResponse(server, 0, 13, "", 0);
                        printf("OK ENVIADO!\n");
                    }
                    else if(message.type == 8) {
                        sendResponse(server, 0, 14, "", 0);
                        printf("ACK ENVIADO!\n");
                    }
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
            file = fopen(filePath, "r");
            if(!file) {
                sendResponse(server, 0, 12, "2", 1);
                printf("erro enviado!\n");
                continue;
            }
            printf("enviando arquivo!");
            sendFile(file, fileName, server, 0);
            fclose(file);
        }
    }
        
    
    return 0;

}