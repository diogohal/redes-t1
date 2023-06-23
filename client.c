#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include "rawSocketConnection.h"
#include "fileHandler.h"
#include "packages.h"

void getCommand(int *command, char *input) {

    char *token = NULL;
    token = strtok(input, " ");
    token[strcspn(token, "\n")] = '\0';
    printf("token:%s\n", token);
    if(!strcmp(token, "backup"))
        *command = 0;
    else if(!strcmp(token, "backupdir"))
        *command = 1;
    else if(!strcmp(token, "rec-backup"))
        *command = 2;
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
    sockfd = rawSocketConnection("eno1");
    int command = -1; char dirPath[200]; char fileName[50];
    
    // Client running
    while (running) {
        // ----- Get terminal command -----
        command = -2;
        cmd[0] = '\0';
        saveCmd[0] = '\0';
        fgets(cmd, sizeof(cmd), stdin);
        strcpy(saveCmd, cmd);
        getCommand(&command, cmd);
        printf("cmd %d\n", command);
        strcpy(cmd, saveCmd);
        // ----- Commands execution -----
        // 1) 1 file backup
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
            sendFile(file, fileName, sockfd, 0);
            fclose(file);
        }
        // 2) Backup files inside folder
        else if(command == 1) {
            getDirPath(dirPath, cmd);
            dirStream = opendir(dirPath);
            if(!dirStream) {
                printf("Diretório inexistente!\n");
                continue;
            }
            sendDirectory(dirPath, sockfd);
        }
        // 3) One file backup recover
        else if(command == 2) {
            getFileName(fileName, cmd, 1);
            sendResponse(sockfd, 0, 2, fileName, strlen(fileName));

            while (1) {
                printf("Entrou no while!!\n");
                recv(sockfd, &message, 67, 0);

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
                    printf("Recebi mensagem %d | data = %s\n", message.sequel, message.data);
                    // Server-Client talk
                    if(message.type == 0) {
                        sendResponse(sockfd, 0, 13, "", 0);
                        printf("OK ENVIADO!\n");
                    }
                    else if(message.type == 8) {
                        sendResponse(sockfd, 0, 14, "", 0);
                        printf("ACK ENVIADO!\n");
                    }
                    // Create a list of messages
                    if (receiveFileMessage(root, message))
                        break;
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