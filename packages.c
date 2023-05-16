#include <stdlib.h>
#include <stdio.h>
#include "packages.h"
#include "string.h"

protocol_t createMessage (unsigned int sequel, unsigned int type, unsigned char data[64]) {
    protocol_t message;
    int bit = 0, size = strlen(data);

    //alocate the init mark on the protocol
    for (int i = 0; i < 8; i++) {
        if (i == 0 || i == 7)
            message.init_mark |= (0 << i);
        else
            message.init_mark |= (1 << i);
    }

    //alocate the size of the message on the protocol
    for (int i = 0; i < 6; i++) {
        bit = size % 2;

        message.size |= (bit << i);

        size >>= 1;
    }

    for (int i = 0; i < 6; i++) {
        message.sequel |= (0 << i);
    }

    for (int i = 0; i < 4; i ++) {
        message.type |= (0 << i);
    }

    strcpy(message.data, data);

    for (int i = 0; i < 8; i++) {
        message.parity |= (0 << i);
    }

    return message;
}

unsigned char* dataToBit (unsigned char data[64]) {
    
    unsigned char buffer[96];

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            mask = 1 << j;
        }
    }
}