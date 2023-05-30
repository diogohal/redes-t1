#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"
#include "packages.h"
#include "fileHandler.h"

int main() {

    int server, new_socket, valread;
    root_t *root = createRoot();
    node_t *auxNode = NULL;
    protocol_t *auxMessage = NULL;
    unsigned char *msg = NULL;
    protocol_t message;
    server = rawSocketConnection("eno1");
    while(1) {
        recv(server, &message, 67, 0);
        if(message.init_mark == 126) {
            // printf("ORDER = %d | %s\n\n\n", message.sequel, message.data);
            // printf("%s", message.data);
            auxMessage = createMessage(message.sequel, message.type, message.data);
            auxNode = createNode(auxMessage);
            addNode(root, auxNode);
            if(message.type == 9) {
                msg = createString(root);
                writeFile(msg);
            }
        }
    }
        
    
    return 0;

}