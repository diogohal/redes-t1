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
    int sockfd = 0;
    int bufferSize = 0;
    struct sockaddr_in dest_addr;
    char *token = NULL;
    DIR *dirStream = NULL;
    struct dirent *dirEntry = NULL;
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
        command = -2;
        fgets(cmd, sizeof(cmd), stdin);
        strcpy(saveCmd, cmd);
        getCommand(&command, cmd);
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
            getFileName(fileName, cmd);
            // Send and close file
            sendFile(file, fileName, sockfd);
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

        // 0) Exit
        else if (command == -1)
            running = 0;
        
        // 404) Error
        else if(command = 404)
            printf("Comando não encontrado!\n");
    }

    return 0;

}