#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"

int main() {

    int server, new_socket, valread;
    
    server = rawSocketConnection("enp0s31f6");
    char *hello = "Hello World from server";

    valread = read(new_socket);
    
    
    return 0;

}