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
    server = rawSocketConnection("lo");

    // Server running
    while(1) {
        // Receive message
        recv(server, &message, 67, 0);

        // File
        if(message.sequel != sequel && message.init_mark == 126 && (message.type == 0 || message.type == 9 || message.type == 8)) {
            printf("Recebi mensagem %d | data = %s\n", message.sequel, message.data);
            sequel = message.sequel;
            // Server-Client talk
            if(message.type != 9) {
                sendACK(server, message.sequel);
                printf("ACK ENVIADO!\n");
            }
            // Create a list of messages
            receiveFileMessage(root, message);
        }

        // Group file
        // if(message.init_mark == 126 && message.type == 1) {
        //     printf("Grupo de arquivos!\n");
        //     sendACK(server);
        //     printf("ACK GRUPO ENVIADO!\n");
        //     while(1) {
        //         recv(server, &message, 67, 0);
        //         // Group file end
        //         if(message.init_mark == 126 && message.type == 10)
        //             break;
                
        //         // File content
        //         while(1) {
        //             recv(server, &message, 67, 0);
        //             if(message.init_mark == 126 && (message.type == 8 || message.type == 9 || message.type == 0)) {
        //                 // Create a list of messages and send it when it's complete
        //                 response = receiveFileMessage(root, message);
        //                 if(response)
        //                     break;
        //                 sendACK(server);
        //             }
        //         }
        //         sendACK(server);
        //     }
        //     printf("Fim grupo de arquivos!\n");
        // }
    }
        
    
    return 0;

}