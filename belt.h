#ifndef _BELT_H_
#define _BELT_H_

#include <pthread.h>
#include "parts.h"

#define debugBelt \
    ((debug & 2) || (debug & 1))

extern pthread_mutex_t beltMutex;
extern pthread_cond_t activeBelt;
extern pthread_t beltThread;
extern unsigned int nSlots;
extern int debug;
extern int runCycles;
extern int maxRunCycles;
extern int runs;
extern int maxRuns;
extern unsigned int productCount;
extern unsigned int wastedA;
extern unsigned int wastedB;
extern unsigned int producedA;
extern unsigned int producedB;

struct beltSlot {
    parts slotPart;
    pthread_mutex_t slotMutex;
    struct beltSlot *prevSlot;
    struct beltSlot *nextSlot;
};

extern struct beltSlot* headSlot;
extern struct beltSlot* tailSlot;

void* belt(void *);
void createBeltSlots(void);
extern struct beltSlot** beltSlotsRef;
void updateTailPartCount(parts);

#endif
