#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rawSocketConnection.h"
#include "packages.h"

#define DEST_PORT 8080
#define DEST_IP "172.18.0.1"

int main() {

    int sockfd;
    char *msg = "hello world";
    int msg_len = (strlen(msg));
    struct sockaddr_in dest_addr;
    protocol_t message = createMessage(0, 0, "Hello World!");
    
    sockfd = rawSocketConnection("enp0s25");

    send(sockfd, message.data, 96, 0);

    // memset(&dest_addr, 0, sizeof(dest_addr));

    // dest_addr.sin_family = AF_INET;
    // dest_addr.sin_port = htons(DEST_PORT);

    // if (inet_aton(DEST_IP, &dest_addr.sin_addr) == 0) {
    //     perror("inet_aton");
    //     exit(1);
    // }

    // if (send(sockfd, msg, 13, 0) < 0) {
    //     perror("send");
    //     printf(" %s\n",strerror(errno));
    //     exit(1);
    // }

    // printf("%d\n", sizeof(char*) * msg_len);
    // close(sockfd);

    return 0;

}