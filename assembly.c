#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "belt.h"
#include "worker.h"

int nParts = 0;
int debug = 0;
int busyCycles = 0;
int runCycles = 0;
int maxRunCycles = 0;
int runs = 0;
int maxRuns = 0;
unsigned int wastedA = 0;
unsigned int wastedB = 0;
unsigned int producedA = 0;
unsigned int producedB = 0;
unsigned int productCount = 0;

int main(int argc, char *argv[]) {
    int i = 0;
    int c;
    char* validOptArg = NULL;
    while (1) {
        int lOptIndex = 0;
        static struct option lOptions[] = {
            {"debug", optional_argument, 0, 'd'},
            {      0,                 0, 0, 0},
        };
        c = getopt_long(argc, argv, "d::", lOptions, &lOptIndex);
        if (c == -1)
            break;
        switch (c) {
            case 'd':
                debug = 1; 
                if (!optarg) {
                    if ((optind < argc) && (argv[optind][0] != '-')) {
                        optarg = argv[optind++];
                    }
                }
                if (optarg) {
                    if (!strcmp(optarg, "belt")) {
                        debug = 2;
                    } else if (!strcmp(optarg, "worker")) {
                        debug = 4;
                    } else if (!strcmp(optarg, "all")) {
                    } else {
                        fprintf(stderr, "%s: invalid option args: %s\n", argv[0], optarg);
                        return 0;
                    }
                    validOptArg = malloc(strlen(optarg));
                    memcpy(validOptArg, optarg, strlen(optarg));
                }
                break;
            case '?':
            default:
                return 0;
        }
    }
    if (optind < argc) {
        fprintf(stderr, "%s: invalid option args:", argv[0]);
        while (optind < argc) {
            fprintf(stderr, " %s", argv[optind++]);
        }
        fprintf(stderr, "\n");
        free(validOptArg);
        return 0;
    }
    printf("setting: debug %s\n", validOptArg?validOptArg:"all");
    free(validOptArg);
    nParts = 3;
    nSlots = nParts;
    maxRuns = 100;
    maxRunCycles = 1;
    runs = 0;
    runCycles = 0;
    busyCycles = nParts;
    nWorkers = 2*nSlots;
    beltSlotsRef = malloc(nSlots*sizeof(struct beltSlotRef*));
    createBeltSlots();
    pthread_mutex_init(&beltMutex, NULL);
    pthread_cond_init(&activeBelt, NULL);
    pthread_cond_init(&activeWorkers, NULL);
    pthread_create(&beltThread, NULL, belt, NULL);
    workerThreads = malloc(nWorkers*sizeof(pthread_t));
    workerNeeds = malloc(nWorkers*sizeof(parts));
    for(i = 0; i < nWorkers; i++) {
        struct worker* w = malloc(sizeof(struct worker));
        w->workerId = i;
        w->hasPart = E;
        w->busyAssembling = 0;
        workerNeeds[i] = X;
        pthread_create(&(workerThreads[i]), NULL, worker, (void *)w);
    }
    pthread_join(beltThread, NULL);
    for(i = 0; i < nWorkers; i++) {
        pthread_join(workerThreads[i], NULL);
    }
    for(i = 0; i < nSlots; i++) {
        free(beltSlotsRef[i]);
    }
    free(beltSlotsRef);
    free(workerThreads);
    free(workerNeeds);
    printf("Part As produced: %d\n", producedA);
    printf("Part Bs produced: %d\n", producedB);
    printf("Completed Products: %d\n", productCount);
    printf("Part As wasted: %d\n", wastedA);
    printf("Part Bs wasted: %d\n", wastedB);
    return 0;
}
