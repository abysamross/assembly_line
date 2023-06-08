#include <stdio.h>
#include <stdlib.h>
#include "worker.h"
#include "belt.h"

pthread_cond_t activeWorkers;
pthread_t* workerThreads = NULL;
parts* workerNeeds;

unsigned int workersStatus = 0;
unsigned int nWorkers;

unsigned int anyWorkerNeedsPart(parts p) {
    int i = 0;
    for (i = 0; i < nWorkers; i++) {
        if (workerNeeds[i] == p) {
            if (debugWorker) {
                fprintf(stderr, "### %-20s ### another worker: %d needs part: %c\n", __FUNCTION__, i, getPartStr(p));
            }
            return 1;
        }
    }
    return 0;
}

unsigned int noWorkerNeedsPart(parts p) {
    return !anyWorkerNeedsPart(p);
}

void putProductToBelt(struct worker *w) {
    unsigned int slotId = w->workerId/2;
    struct beltSlot* slot = beltSlotsRef[slotId];
    pthread_mutex_lock(&(slot->slotMutex));
    parts p = slot->slotPart;
    if (debugWorker) {
        fprintf(stderr, "### %-20s ### worker: %d has part: %c, needs part: %c, belt slot: %d, part: %c\n", __FUNCTION__, w->workerId, getPartStr(w->hasPart), getPartStr(workerNeeds[w->workerId]), slotId, getPartStr(p));
    }

    if (p == E) {
        slot->slotPart = w->hasPart;
        w->hasPart = E;
        workerNeeds[w->workerId] = X;
    }

    if (debugWorker) {
        fprintf(stderr, "### %-20s ### worker: %d has part: %c, needs part: %c, belt slot: %d, part: %c\n", __FUNCTION__, w->workerId, getPartStr(w->hasPart), getPartStr(workerNeeds[w->workerId]), slotId, getPartStr(slot->slotPart));
    }
    pthread_mutex_unlock(&(slot->slotMutex));
}

void getPartFromBelt(struct worker *w) {
    unsigned int slotId = w->workerId/2;
    struct beltSlot* slot = beltSlotsRef[slotId];
    pthread_mutex_lock(&(slot->slotMutex));
    parts p = slot->slotPart;
    if (debugWorker) {
        fprintf(stderr, "### %-20s ### worker: %d has part: %c, needs part: %c, belt slot: %d, part: %c\n", __FUNCTION__, w->workerId, getPartStr(w->hasPart), getPartStr(workerNeeds[w->workerId]), slotId, getPartStr(p));
    }

    if ((p != E) && (p != P)) {
        if ((p == workerNeeds[w->workerId]) || ((workerNeeds[w->workerId] == X) && noWorkerNeedsPart(p))) {
            slot->slotPart = E;
            w->hasPart = p;
            if (workerNeeds[w->workerId] != X) {
                w->hasPart = P; 
            }
            workerNeeds[w->workerId] = counterPart(w->hasPart);
        }
    }

    if (debugWorker) {
        fprintf(stderr, "### %-20s ### worker: %d has part: %c, needs part: %c, belt slot: %d, part: %c\n", __FUNCTION__, w->workerId, getPartStr(w->hasPart), getPartStr(workerNeeds[w->workerId]), slotId, getPartStr(slot->slotPart));
    }
    pthread_mutex_unlock(&(slot->slotMutex));
}

void makeWorkersActive() {
    int i = 0;
    for (i = 0; i < nWorkers; i++) {
        workersStatus |= (1 << i); 
    }
    if (debugBelt) {
        fprintf(stderr, "||| %-20s ||| workersStatus: %x\n", __FUNCTION__, workersStatus);
    }
}

unsigned int isAnyWorkerActive() {
    if (debugBelt) {
        fprintf(stderr, "--- %-20s --- workersStatus: %x\n", __FUNCTION__, workersStatus);
    }
    if (!workersStatus) {
        return 0;
    } 
    return 1;
}

unsigned int isWorkerActive(unsigned int id) {
    unsigned int status = workersStatus;
    status >>= id;
    if (debugWorker) {
        fprintf(stderr, "### %-20s ### worker: %d: status: %u\n", __FUNCTION__, id, status & 1); }
    return status & 1; 
}

void setWorkerInactive(unsigned int id) {
    unsigned int bitMask = ~(1 << id);
    workersStatus &= bitMask; 
    if (debugWorker) {
        fprintf(stderr, "### %-20s ### worker: %d: workersStatus: %x\n", __FUNCTION__, id, workersStatus);
    }
}


void* worker(void* arg) {
    struct worker* w = arg; 
    while(1) {
        pthread_mutex_lock(&beltMutex);
        while(!isWorkerActive(w->workerId)) {
            pthread_cond_wait(&activeBelt, &beltMutex);
        }
        pthread_mutex_unlock(&beltMutex);


        if (!w->busyAssembling) {
            if (debugWorker) {
                fprintf(stderr, "### %-20s ### %d free to work\n", __FUNCTION__, w->workerId);
            }
            if (w->hasPart == P) {
                putProductToBelt(w);
            } else {
                getPartFromBelt(w);
                if (w->hasPart == P) {
                    w->busyAssembling = busyCycles;
                }
            }
        } else {
            if (debugWorker) {
                fprintf(stderr, "### %-20s ### %d busy assembling: %d\n", __FUNCTION__, w->workerId, w->busyAssembling);
            }
            (w->busyAssembling)--;
        }

        pthread_mutex_lock(&beltMutex);
        setWorkerInactive(w->workerId);
        unsigned int signalBelt = isAnyWorkerActive();
        pthread_mutex_unlock(&beltMutex);
        if (!signalBelt) {
            if (debugWorker) {
                fprintf(stderr, "### %-20s ### %d signalling belt\n", __FUNCTION__, w->workerId);
            }
            pthread_cond_signal(&activeWorkers);
        }
        if (runs == maxRuns) {
            printf("worker %d thread exiting\n", w->workerId);
            if (debugWorker) {
                fprintf(stderr, "### %-20s ### %d thread exiting\n", __FUNCTION__, w->workerId);
            }
            parts p = w->hasPart;
            pthread_mutex_lock(&beltMutex);
            if (p != P) {
                updateTailPartCount(p);
            } else {
                updateTailPartCount(A);
                updateTailPartCount(B);
            }
            pthread_mutex_unlock(&beltMutex);
            free(w);
            pthread_exit(NULL);
        }
    }
    return NULL;
}
