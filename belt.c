#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "belt.h"
#include "worker.h"

pthread_mutex_t beltMutex;
pthread_cond_t activeBelt;
pthread_t beltThread;
unsigned int nSlots = 0;
struct beltSlot* headSlot = NULL;
struct beltSlot* tailSlot = NULL;
struct beltSlot** beltSlotsRef = NULL;

void updateProducedCount(parts p) {
    switch (p) {
        case A:
            producedA++;
            break;
        case B:
            producedB++;
            break;
        case E:
        case P:
        case X:
            break;
    }
}

void updateTailPartCount(parts p) {
    switch (p) {
        case A:
            wastedA++;
            break;
        case B:
            wastedB++;
            break;
        case P:
            productCount++;
            break;
        case E:
        case X:
            break;
    }
}

void printUsingBeltSlotsRef() {
    int i = 0;
    fprintf(stderr, "\n\n");
    for (i = 0; i < nSlots; i++) {
        struct beltSlot* beltSlot = beltSlotsRef[i];
        fprintf(stderr, "%c", getPartStr(beltSlot->slotPart));
        if (beltSlot->nextSlot != NULL) {
            fprintf(stderr, " --> ");
        }
    }
    fprintf(stderr, "\n\n");
}

void printBeltSlots() {
    struct beltSlot* itr = headSlot;
    printf("\n\n");
    while (itr != NULL) {
        printf("%c", getPartStr(itr->slotPart));
        if (itr->nextSlot != NULL) {
            printf(" --> ");
        }
        itr = itr->nextSlot;
    }
    printf("\n\n");
}

void reverseBeltSlotsRef(int i, int j) {
    for ( ; i < j; i++, j--) {
        struct beltSlot* temp = beltSlotsRef[i];
        beltSlotsRef[i] = beltSlotsRef[j];
        beltSlotsRef[j] = temp;
    }
}

void updateBeltSlotsRef() {
    reverseBeltSlotsRef(0, nSlots - 2);
    reverseBeltSlotsRef(0, nSlots - 1);
}

void updateBeltSlots(parts p) {
    updateTailPartCount(tailSlot->slotPart);
    if (headSlot == tailSlot) {
        headSlot->slotPart = p;
        return;
    }
    tailSlot->slotPart = p;
    tailSlot->nextSlot = headSlot;
    headSlot->prevSlot = tailSlot;
    tailSlot->prevSlot->nextSlot = NULL;
    headSlot = tailSlot;
    tailSlot = headSlot->prevSlot;
    headSlot->prevSlot = NULL;
    updateBeltSlotsRef();
}

void createBeltSlots(void) {
    int i = 0;
    for (i = 0; i < nSlots; i++) {
        struct beltSlot* slot = malloc(sizeof(struct beltSlot));
        if (!headSlot) {
            headSlot = slot;
            slot->prevSlot = NULL;
        }
        slot->slotPart = E;
        pthread_mutex_init(&(slot->slotMutex), NULL);
        slot->nextSlot = NULL;
        if (tailSlot) {
            tailSlot->nextSlot = slot;
            slot->prevSlot = tailSlot;
        }
        tailSlot = slot;
        beltSlotsRef[i] = slot;
    }
}

parts generatePart() {
    struct timeval tv;
    unsigned int seed = 1;
    if (!gettimeofday(&tv, NULL)) {
        seed = (unsigned int)tv.tv_usec;
    }
    srand(seed);
    return (parts)(rand() % nParts);
}

void* belt(void* arg) {
    while (1) {
        pthread_mutex_lock(&beltMutex);
        while (isAnyWorkerActive()) {
            pthread_cond_wait(&activeWorkers, &beltMutex);
        }
        if (runs == maxRuns) {
            int i = 0;
            for (i = 0; i < nSlots; i++) {
                struct beltSlot* beltSlot = beltSlotsRef[i];
                updateTailPartCount(beltSlot->slotPart);
            }
            printf("belt thread exiting\n");
            if (debugBelt) {
                fprintf(stderr, "||| %-20s ||| exiting\n", __FUNCTION__);
            }
            pthread_mutex_unlock(&beltMutex);
            pthread_exit(NULL);
        } else {
            runCycles++;
            parts p = generatePart();
            updateProducedCount(p);
            updateBeltSlots(p);
            printBeltSlots();
            if (debugBelt) {
                printUsingBeltSlotsRef();
            }
            makeWorkersActive();
            if (runCycles == maxRunCycles) {
                runCycles = 0;
                if (debugBelt) {
                    fprintf(stderr, "||| %-20s ||| run: %d done\n", __FUNCTION__, runs);
                }
                runs++;
            }
            pthread_mutex_unlock(&beltMutex);
            if (debugBelt) {
                fprintf(stderr, "||| %-20s ||| broadcasting signal to workers\n", __FUNCTION__);
            }
            pthread_cond_broadcast(&activeBelt);
        }
    }
    return NULL;
}
