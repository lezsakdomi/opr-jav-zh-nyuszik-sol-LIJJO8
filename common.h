//
// Created by led on 5/18/22.
//

#ifndef OPR_ZH_COMMON_H
#define OPR_ZH_COMMON_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <mqueue.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

int verbosity = 5;
int tapsiPid, fulesPid;
int num_interrupt = 0;
int expectedNumInterrupt = 0;

enum OperationResult {
    OPR_SUCCESS = 0,
    OPR_CALL_FAILED = 1,
    OPR_PROTOCOL_ERROR = 2,
    OPR_OUT_OF_BOUNDS,
    OPR_NO_MATCH,
    OPR_NOT_IMPLEMENTED,
    OPR_IO_ERROR,
    OPR_FILE_ERROR,
    OPR_END_OF_FILE,
    OPR_BAD_ARGUMENT,
    OPR_FORK_FAILED,
};

#define call(FUN) do { \
    if (errno != 0 && verbosity >= 1) { \
        fprintf(stderr, "Warning: errno already set at %s:%d: %s", \
            __FILE__, __LINE__, strerror(errno)); \
        errno = 0; \
    } \
    enum OperationResult callResult = (FUN); \
    if (callResult) { \
        if (verbosity >= 1) { \
            if (callResult != OPR_CALL_FAILED) { \
                fprintf(stderr, "Error #%d (errno = %s) in %s:%d\n", \
                    callResult, strerror(errno), __FILE__, __LINE__); \
            } \
            fprintf(stderr, "  in call %s\n", #FUN); \
        } \
        return OPR_CALL_FAILED; \
    } \
} while (0)

#define throw do { \
    fprintf(stderr, "Error thrown from %s:%d (errno = %s)\n", \
        __FILE__, __LINE__, strerror(errno)); \
    return OPR_CALL_FAILED;                                                   \
} while (0)

#define pprintf(format, ...) printf("[process %d] " format, getpid() __VA_OPT__(,) __VA_ARGS__)
#define sappend(s, format, ...) sprintf(s + strlen(s), format __VA_OPT__(,) __VA_ARGS__)

enum OperationResult urlEncode(char *s, char *r) {
    for (; *s; s++) {
        if (isalnum(*s) || *s == '-' || *s == '_') *r = *s;
        else sprintf(r, "%%%02X", *s);
        for (; *r; r++);
    }

    return OPR_SUCCESS;
}

int interruptHandlerLocked = 0;

void interruptHandler(int sig, siginfo_t *info, void *ucontext) {
    while (interruptHandlerLocked) {
        pprintf("Waiting for signal interrupt lock...\n");
        sleep(0);
    }
    interruptHandlerLocked = 1;

    if (verbosity >= 1) {
        pprintf("Received signal %d (%s) from process %d\n",
                sig, strsignal(sig), info->si_pid);
    }

    if (verbosity >= 8) {
        char msg[256];
        sprintf(msg, "[process %d] [verbose] received signal", getpid());
        psiginfo(info, msg);
    }

    if (verbosity >= 9) {
        pprintf("[signal] Signal number: %d\n", info->si_signo);
        pprintf("[signal] An errno value: %d\n", info->si_errno);
        pprintf("[signal] Signal code: %d\n", info->si_code);
//    pprintf("[signal] Trap number that caused hardware-generated signal: %d\n", info->si_trapno);
        pprintf("[signal] Sending process ID: %d\n", info->si_pid);
        pprintf("[signal] Real user ID of sending process: %d\n", info->si_uid);
        pprintf("[signal] Exit value or signal: %d\n", info->si_status);
        pprintf("[signal] User time consumed: %ld\n", info->si_utime);
        pprintf("[signal] System time consumed: %ld\n", info->si_stime);
//    pprintf("[signal] Signal value: %d\n", info->si_value);
        pprintf("[signal] POSIX.1b signal: %d\n", info->si_int);
        pprintf("[signal] POSIX.1b signal: %lx\n", (long) info->si_ptr);
        pprintf("[signal] Timer overrun count; POSIX.1b timers: %d\n", info->si_overrun);
        pprintf("[signal] Timer ID; POSIX.1b timers: %d\n", info->si_timerid);
        pprintf("[signal] Memory location which caused fault: %lx\n", (long) info->si_addr);
        pprintf("[signal] Band event: %ld\n", info->si_band);
        pprintf("[signal] File descriptor: %d\n", info->si_fd);
        pprintf("[signal] Least significant bit of address: %d\n", info->si_addr_lsb);
        pprintf("[signal] Lower bound when address violation occurred: %lx\n", (long) info->si_lower);
        pprintf("[signal] Upper bound when address violation occurred: %lx\n", (long) info->si_upper);
        pprintf("[signal] Protection key on PTE that caused fault: %d\n", info->si_pkey);
        pprintf("[signal] Address of system call instruction: %lx\n", (long) info->si_call_addr);
        pprintf("[signal] Number of attempted system call: %d\n", info->si_syscall);
        pprintf("[signal] Architecture of attempted system call: %d\n", info->si_arch);
//        int      si_signo;     /* Signal number */
//        int      si_errno;     /* An errno value */
//        int      si_code;      /* Signal code */
//        int      si_trapno;    /* Trap number that caused hardware-generated signal */
//        pid_t    si_pid;       /* Sending process ID */
//        uid_t    si_uid;       /* Real user ID of sending process */
//        int      si_status;    /* Exit value or signal */
//        clock_t  si_utime;     /* User time consumed */
//        clock_t  si_stime;     /* System time consumed */
//        union sigval si_value; /* Signal value */
//        int      si_int;       /* POSIX.1b signal */
//        void    *si_ptr;       /* POSIX.1b signal */
//        int      si_overrun;   /* Timer overrun count; POSIX.1b timers */
//        int      si_timerid;   /* Timer ID; POSIX.1b timers */
//        void    *si_addr;      /* Memory location which caused fault */
//        long     si_band;      /* Band event */
//        int      si_fd;        /* File descriptor */
//        short    si_addr_lsb;  /* Least significant bit of address */
//        void    *si_lower;     /* Lower bound when address violation occurred */
//        void    *si_upper;     /* Upper bound when address violation occurred */
//        int      si_pkey;      /* Protection key on PTE that caused fault */
//        void    *si_call_addr; /* Address of system call instruction */
//        int      si_syscall;   /* Number of attempted system call */
//        unsigned int si_arch;  /* Architecture of attempted system call */
    }
    if (sig == SIGUSR1) { num_interrupt++; }

    interruptHandlerLocked = 0;
}

enum OperationResult waitInterrupt(const pid_t *expectedSender) {
    expectedNumInterrupt++;

    // todo use sigsuspend
    while (num_interrupt < expectedNumInterrupt) {
        if (errno) throw;
//        pause();
        sleep(0);
        errno = 0;
        if (expectedSender) kill(*expectedSender, SIGCONT);
    }

    return OPR_SUCCESS;
}

enum PipeEnd {
    PIPE_END_UNDEFINED = 0,
    PIPE_END_NONE,
    PIPE_END_UPPER,
    PIPE_END_LOWER,
};

struct DuplexPipe {
    int upstream[2];
    int downstream[2];
    enum PipeEnd selectedEnd;
};

enum OperationResult initializePipe(struct DuplexPipe *dPipe);

enum OperationResult selectPipeEnd(struct DuplexPipe *dPipe, enum PipeEnd selectedEnd);

enum OperationResult readPipe(struct DuplexPipe dPipe, void *buf, size_t length);

enum OperationResult writePipe(struct DuplexPipe dPipe, void *buf, size_t length);

enum OperationResult closePipe(struct DuplexPipe dPipe);

enum OperationResult initializePipe(struct DuplexPipe *dPipe) {
    call(pipe(dPipe->downstream));
    call(pipe(dPipe->upstream));
    dPipe->selectedEnd = PIPE_END_UNDEFINED;

    return OPR_SUCCESS;
}

enum OperationResult selectPipeEnd(struct DuplexPipe *dPipe, enum PipeEnd selectedEnd) {
    if (dPipe->selectedEnd != PIPE_END_UNDEFINED) return OPR_BAD_ARGUMENT;

    dPipe->selectedEnd = selectedEnd;
    switch (selectedEnd) {
        case PIPE_END_NONE:
            call(close(dPipe->downstream[0]));
            call(close(dPipe->downstream[1]));
            call(close(dPipe->upstream[0]));
            call(close(dPipe->upstream[1]));
            break;

        case PIPE_END_UPPER:
            call(close(dPipe->downstream[0]));
            call(close(dPipe->upstream[1]));
            break;

        case PIPE_END_LOWER:
            call(close(dPipe->upstream[0]));
            call(close(dPipe->downstream[1]));
            break;

        default:
            return OPR_BAD_ARGUMENT;
    }
}

enum OperationResult readPipe(struct DuplexPipe dPipe, void *buf, size_t length) {
    int fd;

    switch (dPipe.selectedEnd) {
        case PIPE_END_UNDEFINED:
        case PIPE_END_NONE:
            return OPR_BAD_ARGUMENT;

        case PIPE_END_UPPER:
            fd = dPipe.upstream[0];
            break;

        case PIPE_END_LOWER:
            fd = dPipe.downstream[0];
            break;
    }

    ssize_t readLength = read(fd, buf, length);
    if (readLength != length) return OPR_IO_ERROR;
    else return OPR_SUCCESS;
}

enum OperationResult writePipe(struct DuplexPipe dPipe, void *buf, size_t length) {
    int fd;

    switch (dPipe.selectedEnd) {
        case PIPE_END_UNDEFINED:
        case PIPE_END_NONE:
            return OPR_BAD_ARGUMENT;

        case PIPE_END_UPPER:
            fd = dPipe.downstream[1];
            break;

        case PIPE_END_LOWER:
            fd = dPipe.upstream[1];
            break;
    }

    ssize_t writtenLength = write(fd, buf, length);
    if (writtenLength != length) return OPR_IO_ERROR;
    else return OPR_SUCCESS;
}

enum OperationResult closePipe(struct DuplexPipe dPipe) {
    int fd;

    switch (dPipe.selectedEnd) {
        case PIPE_END_UNDEFINED:
        case PIPE_END_NONE:
            return OPR_BAD_ARGUMENT;

        case PIPE_END_UPPER:
            fd = dPipe.downstream[1];
            break;

        case PIPE_END_LOWER:
            fd = dPipe.upstream[1];
            break;
    }

    call(close(fd));

    return OPR_SUCCESS;
}

const int NUM_REGIONS;
const char *const REGION_NAMES[7];

enum Region {
    UNDEFINED_REGION = -1,
    BARATFA,
    LOVAS,
    SZULA,
    KIGYOS_PATAK,
    MALOM_TELEK,
    PASKOM,
    KAPOSZTAS_KERT,
};

enum OperationResult regionEnumToName(enum Region id, const char **name);

enum OperationResult regionNameToEnum(const char *name, enum Region *id);

enum OperationResult regionEnumToMask(enum Region id, int* mask);

const int NUM_REGIONS = 7;
const char *const REGION_NAMES[] = {
        "Barátfa",
        "Lovas",
        "Szula",
        "Kígyós-patak",
        "Malom telek",
        "Páskom",
        "Káposztás kert",
};

enum OperationResult regionEnumToName(enum Region id, const char **name) {
    if (0 > id || NUM_REGIONS < id) {
        return OPR_NO_MATCH;
    } else {
        *name = REGION_NAMES[id];
        return OPR_SUCCESS;
    }
}

enum OperationResult regionNameToEnum(const char *name, enum Region *id) {
    for (*id = 0; *id < NUM_REGIONS; (*id)++) {
        const char* currentName;
//        printf("[%d/%d] ", *id, NUM_REGIONS);
        call(regionEnumToName(*id, &currentName));
//        printf("%s =? %s\n", name, currentName);
        if (strcmp(name, currentName) == 0) {
            return OPR_SUCCESS;
        }
    }

    return OPR_NO_MATCH;
}

enum OperationResult regionEnumToMask(enum Region id, int *mask) {
    *mask = 1 << id;
    return OPR_SUCCESS;
}

struct DuplexPipe tapsiPipe, fulesPipe;

#define toMask(enumVal) (1 << (enumVal))

enum Command {
    CMD_TRANSFER_MASK,
    CMD_TRANSFER_RESULT,
    CMD_TRANSFER_FINISHED,
};

struct Msg {
    enum Command command;
    int region;
    int data;
};

enum EVALR {
        EVALR_SAME,
        EVALR_LESS,
        EVALR_MORE,
};

char* evalr_str[3] = {
        "unknown situation",
        "nyuszifogyás",
        "nyuszigyarapodás",
};

#define Hazaterhet SIGUSR1
#define Keress_meg SIGUSR2

//struct shmbuf {
//    sem_t  sem1; // can write
//    sem_t  sem2; // write done
//    int data;
//};

#endif //OPR_ZH_COMMON_H
