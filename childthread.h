//
// Created by led on 5/18/22.
//

#ifndef OPR_ZH_CHILDTHREAD_H
#define OPR_ZH_CHILDTHREAD_H

#include "common.h"

enum OperationResult childThread(struct DuplexPipe pipe) {
    if (verbosity >= 1) pprintf("Initializing process...\n");
    struct Msg msg;
    srand(time(NULL) + getpid());
    call(selectPipeEnd(&pipe, PIPE_END_LOWER));
    if (verbosity >= 1) pprintf("Initialized process.\n");
    if (verbosity >= 2) pprintf("Pausing...\n");
    raise(SIGSTOP);
    if (verbosity >= 2) pprintf("Resumed\n");
    call(kill(getppid(), SIGUSR1));

    if (verbosity >= 4) pprintf("Receiving msg...\n");
    call(readPipe(pipe, &msg, sizeof msg));
    int handledRegions = msg.region;
    int friendlyThreadNum = msg.data;

    for (int i = 0; i < NUM_REGIONS; i++) {
        int currentMask;
        call(regionEnumToMask(i, &currentMask));
        if (currentMask & handledRegions) {
            const char *regionName;
            call(regionEnumToName(i, &regionName));
            pprintf("Handling region %s (%d)\n", regionName, friendlyThreadNum);
        }
    }

    int sum = 0;
    for (int i = 0; i < NUM_REGIONS; i++) {
        int currentMask;
        call(regionEnumToMask(i, &currentMask));
        if (currentMask & handledRegions) {
            const char *regionName;
            call(regionEnumToName(i, &regionName));
            int result = rand() % (60 - 100 + 1) + 60;
            sum += result;
            pprintf("Result on region %s: %d\n", regionName, result);
            msg = (struct Msg) {CMD_TRANSFER_RESULT, i, result};
            call(writePipe(pipe, &msg, sizeof msg));
        }
    }
    msg = (struct Msg) {CMD_TRANSFER_FINISHED, handledRegions, sum};
    call(writePipe(pipe, &msg, sizeof msg));

    closePipe(pipe);

    return OPR_SUCCESS;
}

#endif //OPR_ZH_CHILDTHREAD_H
