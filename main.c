//
// Created by led on 5/9/22.
//

#include "common.h"
#include "mainthread.h"
#include "childthread.h"

enum OperationResult createThreads() {
    tapsiPid = fork();
    if (tapsiPid < 0) return OPR_FORK_FAILED;
    if (tapsiPid == 0) {
        call(childThread(tapsiPipe));
        return OPR_SUCCESS;
    }

    fulesPid = fork();
    if (fulesPid < 0) return OPR_FORK_FAILED;
    if (fulesPid == 0) {
        call(childThread(fulesPipe));
        return OPR_SUCCESS;
    }

    call(fonyusziThread());
    return OPR_SUCCESS;
}

int main(int argc, char **argv) {
    initializePipe(&tapsiPipe);
    initializePipe(&fulesPipe);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sa.sa_sigaction = interruptHandler;
    call(sigaction(SIGUSR1, &sa, NULL));
    call(createThreads());
    return 0;
}
