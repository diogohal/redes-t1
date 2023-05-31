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
    root_t *root = createRoot();
    node_t *auxNode = NULL;
    protocol_t *auxMessage = NULL;
    unsigned char *msg = NULL;
    protocol_t message;
    server = rawSocketConnection("lo");

    // Server running
    while(1) {
        // Receive message and check init_mark
        recv(server, &message, 67, 0);
        if(message.init_mark == 126) {
            // Create a list of messages
            auxMessage = createMessage(message.sequel, message.type, message.data);
            auxNode = createNode(auxMessage);
            addNode(root, auxNode);
            printf("Recebi mensagem %d\n", message.sequel);
            // Check for message ending. Needs a timestamp
            if(message.type == 9) {
                msg = createString(root);
                printf("NOME DO ARQUIVO = %s\n", root->head->message->data);
                writeFile(msg, root->head->message->data);
            }
        }
    }
        
    
    return 0;

}