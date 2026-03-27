/**********************************************
 * dwq_client.c — client process
 *
 * Like a Binder client: mmap /dev/dwq (SAME physical page as server),
 * write a request into shared memory, wait for server to mark DONE.
 *
 * Compile: gcc -o dwq_client dwq_client.c
 * Run:     ./dwq_client [cmd] [data]
 *   e.g.:  ./dwq_client 2 "hello server"
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "dwq_uapi.h"

struct dwq_shm {
    unsigned int seq;
    unsigned int handle;
    unsigned int cmd;
    unsigned int state;
    char         data[DWQ_MAX_DATA];
    char         reply_data[DWQ_MAX_DATA];
};

#define DWQ_SHM_IDLE     0
#define DWQ_SHM_PENDING  1
#define DWQ_SHM_DONE     2

int main(int argc, char *argv[])
{
    int fd;
    struct dwq_shm *shm;
    unsigned int cmd  = (argc > 1) ? (unsigned int)atoi(argv[1]) : DWQ_CMD_PING;
    const char   *msg = (argc > 2) ? argv[2] : "hello from client";
    int timeout_ms = 5000;

    fd = open("/dev/dwq", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    /* map the SAME physical page the server has mapped */
    shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

    printf("[client] pid=%d  shm mapped at %p\n", getpid(), shm);

    /* ---- fill transaction (like Binder BC_TRANSACTION) ---- */
    shm->handle = DWQ_HANDLE_DEFAULT;
    shm->cmd    = cmd;
    strncpy(shm->data, msg, DWQ_MAX_DATA - 1);
    shm->data[DWQ_MAX_DATA - 1] = '\0';
    shm->reply_data[0] = '\0';

    /* increment sequence number to mark this as a new request */
    shm->seq++;

    /* publish: set PENDING so server sees it */
    __sync_synchronize();
    shm->state = DWQ_SHM_PENDING;

    printf("[client] sent  seq=%u  cmd=%u  data=\"%s\"\n",
           shm->seq, shm->cmd, shm->data);

    /* ---- wait for server reply (like Binder BR_REPLY) ---- */
    printf("[client] waiting for reply...\n");
    while (timeout_ms > 0) {
        __sync_synchronize();
        if (shm->state == DWQ_SHM_DONE)
            break;
        usleep(1000);
        timeout_ms--;
    }

    if (shm->state == DWQ_SHM_DONE) {
        printf("[client] reply: \"%s\"\n", shm->reply_data);
        shm->state = DWQ_SHM_IDLE;     /* reset for next transaction */
    } else {
        printf("[client] timeout waiting for reply\n");
    }

    munmap(shm, 4096);
    close(fd);
    return 0;
}
