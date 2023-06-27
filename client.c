#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <glob.h>
#include "rawSocketConnection.h"
#include "fileHandler.h"
#include "packages.h"

void getCommand(int *command, char *input) {

    char *token = NULL;
    token = strtok(input, " ");
    token[strcspn(token, "\n")] = '\0';
    if(!strcmp(token, "backup"))
        *command = 0;
    else if(!strcmp(token, "backupg"))
        *command = 1;
    else if(!strcmp(token, "rec-backup"))
        *command = 2;
    else if(!strcmp(token, "rec-backupg"))
        *command = 3;
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

void getFileName(char *fileName, char *input, int op) {

    char *token = NULL;
    
    if (op == 0) {
        token = strtok(input, "/");
        while(token) {
            strcpy(fileName, token);
            fileName[strcspn(fileName, "\n")] = '\0';
            token = strtok(NULL, "/");
        }
    }

    else if (op == 1) {
        token = strtok(input, " ");
        token = strtok(NULL, " ");
        strcpy(fileName, token);
        fileName[strcspn(fileName, "\n")] = '\0';
    }
}

int main(int argc, char** argv) {

    // Variables and structs used in the client
    int sockfd = 0;
    int bufferSize = 0;
    struct sockaddr_in dest_addr;
    char *token = NULL;
    DIR *dirStream = NULL;
    struct dirent *dirEntry = NULL;
    FILE *file = NULL;
    size_t fileSize;
    protocol_t message;
    protocol_t **messageBuffer = NULL;
    root_t *root = createRoot();
    int running = 1;
    int count = 0;
    unsigned char *msg = NULL;
    char cmd[100];
    char saveCmd[100];
    int command = -1; char dirPath[200]; char fileName[50]; char groupPath[50];
    unsigned int parity = 0;
    sockfd = rawSocketConnection("eno1");
    // Timeout setting
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
        printf("Erro ao abrir o socket!\n");
        exit(1);
    }

    ssize_t recvReturn;
    // Client running
    while (running) {
        // ----- Get terminal command -----
        command = -2;
        cmd[0] = '\0';
        saveCmd[0] = '\0';
        dirPath[0] = '\0';
        printf("Comando: ");
        fgets(cmd, sizeof(cmd), stdin);
        strcpy(saveCmd, cmd);
        getCommand(&command, cmd);
        strcpy(cmd, saveCmd);
        // ----- Commands execution -----
        // 0) backup
        if (command == 0) {
            getDirPath(dirPath, cmd);
            file = fopen(dirPath, "rb");
            if(!file) {
                printf("Arquivo ou diretório inexistente!\n");
                continue;
            }
            // Get fileName and it's content
            strcpy(cmd, saveCmd);
            getFileName(fileName, cmd, 0);
            // Send and close file
            printf("Enviando arquivo %s\n", fileName);
            sendFile(file, fileName, sockfd, 0);
            printf("Arquivo enviado!\n");
            fclose(file);
        }
        // 1) backupg
        else if(command == 1) {
            getDirPath(dirPath, cmd);
            sendGroupFiles(dirPath, sockfd);
        }
        // 2) rec-backup
        else if(command == 2) {
            getFileName(fileName, cmd, 1);
            sendResponse(sockfd, 0, 2, fileName, strlen(fileName));

            while (1) {
                recvReturn = recv(sockfd, &message, PROTOCOL_SIZE, 0);
                if(recvReturn == -1) {
                    printf("Timeout! Esperando 2 segundos...");
                    continue;
                }     
                if(message.init_mark == 126 && message.type == 12) {
                    printf("Entrou no erro!!\n");
                    if (!strcmp(message.data, "0"))
                        printf("O Disco está cheio!\n");
                    else if (!strcmp(message.data, "1"))
                        printf("Sem permissão de escrita para o arquivo!\n");
                    else if (!strcmp(message.data, "2"))
                        printf("Arquivo inexistente!\n");
                    else if (!strcmp(message.data, "3"))
                        printf("Sem permissão de leitura para o arquivo!\n");
                    break;
                }
                // ----- File -----
                if(message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8)) {                  
                    // Server-Client talk
                    if(message.type == 0)
                        sendResponse(sockfd, 0, 13, "", 0);
                    else if(message.type == 8)
                        sendResponse(sockfd, 0, 14, "", 0);
                    // Create a list of messages
                    if (receiveFileMessage(root, message))
                        break;
                }
            }
            // rec-backupdir
        } 
        // 3) rec-backupg
        else if (command == 3) {
            getDirPath(dirPath, cmd);
            strcpy(groupPath, "./backup/");
            strcat(groupPath, dirPath);
            sendResponse(sockfd, 0, 3, groupPath, strlen(groupPath)+1);
            while(1) {
                recvReturn = recv(sockfd, &message, PROTOCOL_SIZE, 0);
                if(recvReturn == -1) {
                    printf("Timeout! Esperando 2 segundos...");
                    continue;
                }           
                // File
                if(message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8 || message.type == 10)) {
                    // Server-Client talk
                    if(message.type == 0)
                        sendResponse(sockfd, 0, 13, "", 0);
                    else if(message.type == 8)
                        sendResponse(sockfd, 0, 14, "", 0);
                    else if(message.type == 10)
                        break;
                    // Create a list of messages
                    receiveFileMessage(root, message);
                }
            }
        }
        // 0) Exit
        else if (command == -1)
            running = 0;
        
        // 404) Error
        else if(command = 404)
            printf("Comando não encontrado!\n");
    }

    return 0;

}