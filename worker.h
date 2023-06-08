#ifndef _WORKER_H_
#define _WORKER_H_

# include <pthread.h>
# include "parts.h"
# include "worker.h"

#define debugWorker \
    ((debug & 4) || (debug & 1))

extern pthread_cond_t activeWorkers;
extern unsigned int workerStatus;
extern unsigned int nWorkers;
extern parts* workerNeeds;
extern pthread_t* workerThreads;
extern int busyCycles;

struct worker {
    unsigned int workerId;
    int busyAssembling;
    parts hasPart;
};

unsigned int isAnyWorkerActive();
void makeWorkersActive();
void* worker(void*);

#endif
