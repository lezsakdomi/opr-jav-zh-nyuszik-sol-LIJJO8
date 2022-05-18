//
// Created by led on 5/18/22.
//

#ifndef OPR_ZH_MAINTHREAD_H
#define OPR_ZH_MAINTHREAD_H

#include "common.h"

enum OperationResult fonyusziThread() {
    if (verbosity >= 1) pprintf("Children: Tapsi is %d, Fules is %d\n", tapsiPid, fulesPid);
    srand(time(NULL) + getpid());

    if (verbosity >= 2) pprintf("Waiting for first interrupt...\n");
    call(waitInterrupt(&tapsiPid));
    if (verbosity >= 2) pprintf("Waiting for second interrupt...\n");
    call(waitInterrupt(&fulesPid));
    if (verbosity >= 1) pprintf("Ready\n");

    struct Msg msg;

    call(selectPipeEnd(&tapsiPipe, PIPE_END_UPPER));
    call(selectPipeEnd(&fulesPipe, PIPE_END_UPPER));

    msg = (struct Msg) {
            CMD_TRANSFER_MASK,
            toMask(BARATFA) | toMask(LOVAS) | toMask(KIGYOS_PATAK),
            1,
    };
    call(writePipe(tapsiPipe, &msg, sizeof msg));

    msg = (struct Msg) {
            CMD_TRANSFER_MASK,
            (2 << NUM_REGIONS) - 1 - msg.region,
            2,
    };
    call(writePipe(fulesPipe, &msg, sizeof msg));

    if (verbosity >= 1) pprintf("Messages sent\n");
    sleep(0);

    int results[NUM_REGIONS];
    for (int i = 0; i < NUM_REGIONS; i++) {
        results[i] = rand() % (100 - 50 + 1) + 50;
        if (verbosity >= 1)
        pprintf("At region #%d originally: %d\n",
                i, results[i]);
    }

    for (int i = 0; i < 2; i++) {
        struct DuplexPipe pipe;
        pipe = i ? tapsiPipe : fulesPipe;
        pid_t pid = i ? tapsiPid : fulesPid;

        for (;;) {
            call(readPipe(pipe, &msg, sizeof msg));
            if (msg.command == CMD_TRANSFER_FINISHED) {
                if (verbosity >= 1)
                pprintf("Received end of transfer for 0x%02x (%d total)\n",
                        msg.region, msg.data);
                break;
            } else if (msg.command == CMD_TRANSFER_RESULT) {
                pprintf("Received result for region #%d: %d\n",
                        msg.region, msg.data);
                enum EVALR evalr = EVALR_SAME;
                if (results[msg.region] > msg.data) evalr = EVALR_LESS;
                if (results[msg.region] < msg.data) evalr = EVALR_MORE;
                pprintf("Evaluation: It's a %s\n", evalr_str[evalr]);
                if (evalr == EVALR_LESS) {
//                    int fd = shm_open("/LIJJO8", O_RDWR, 0);
//                    if (fd == -1) throw;
//                    struct shmbuf *shmp = mmap(NULL, sizeof(*shmp),
//                                               PROT_READ | PROT_WRITE,
//                                               MAP_SHARED, fd, 0);
//                    if (shmp == MAP_FAILED) throw;
//
//                    if (sem_post(&shmp->sem1) == -1) throw;
//                    if (sem_wait(&shmp->sem2) == -1) throw;
//                    // todo

                    kill(pid, Keress_meg);
                } else {
                    kill(pid, Hazaterhet); // noop
                }
                results[msg.region] = msg.data;
            } else
                throw;
        }
    }

    waitpid(tapsiPid, NULL, 0);
    pprintf("Tapsi done\n");
    waitpid(fulesPid, NULL, 0);
    pprintf("Fules done\n");

    call(closePipe(tapsiPipe));
    call(closePipe(fulesPipe));

    if (verbosity >= 2) pprintf("Main process cleaned up\n");

    return OPR_SUCCESS;
}

#endif //OPR_ZH_MAINTHREAD_H
