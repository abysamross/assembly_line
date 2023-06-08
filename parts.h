#ifndef _PARTS_H_
#define _PARTS_H_

typedef enum partTypes {
    A,
    B,
    E,
    P,
    X
} parts;

static inline char getPartStr(parts p) {
    char c = '\0';
    switch (p) {
        case A:
            c = 'A';
            break;
        case B:
            c = 'B';
            break;
        case E:
            c = 'E';
            break;
        case P:
            c = 'P';
            break;
        case X:
            c = 'X';
            break;
        default:
            c = '\0';
    } 
    return c;
}

static inline parts counterPart(parts p) {
    parts counter; 
    switch (p) {
        case A:
            counter = B;
            break;
        case B:
            counter = A;
            break;
        case E:
        case X:
            counter = X;
            break;
        case P:
            counter = E;
            break;
        default:
            counter = p;
    }
    return counter;
}

extern int nParts; 

#endif
