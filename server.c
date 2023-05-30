#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"
#include "packages.h"

int main() {

    int server, new_socket, valread;
    protocol_t message;
    server = rawSocketConnection("lo");
    while(1) {
        recv(server, &message, 67, 0);
        if(message.init_mark == 126)
            printf("ORDER = %d | %s\n\n\n", message.sequel, message.data);
    }
        
    
    return 0;

}