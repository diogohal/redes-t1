#ifndef PACKAGES_H
#define PACKAGES_H

struct protocol {
    unsigned int init_mark : 8;
    unsigned int size : 6;
    unsigned int sequel : 6;
    unsigned int type : 4;
    unsigned char data[63];
    unsigned int parity : 8;
}; typedef struct protocol protocol_t;

#endif