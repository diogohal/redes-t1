#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"
#include "packages.h"
#include "fileHandler.h"

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

    }
        
    
    return 0;

}